// =====================================================
// ESP32 + DS18B20 + MQ-2 + HC-SR04 + Servo + Buzzer + ntfy.sh
// Auto-tuned MQ2: delta OR percent OR absolute-mV, with moving baseline.
// =====================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* NTFY_TOPIC = "sensor_project_alerts";
String ntfyUrl() { return "https://ntfy.sh/" + String(NTFY_TOPIC); }

const unsigned long CD_ANY_MS   = 2000;
const unsigned long CD_EVENT_MS = 15000;
unsigned long lastAny = 0, lastGas = 0, lastTemp = 0, lastDist = 0, lastDoor = 0;

#define DS18B20_PIN   32
#define GAS_PIN       35      // MQ-2 AO -> GPIO35
#define BUZZER_PIN    25
#define SERVO_PIN      4
#define BTN_OPEN       5
#define BTN_CLOSE     26
#define TRIG_PIN      12
#define ECHO_PIN      14
#define LED_TEMP       2
#define LED_GAS_A     15
#define LED_GAS_B     13
#define LED_DIST      27

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
Servo myServo;

float TEMP_C_THRESH   = 39.0;
const int DIST_NEAR_MAX = 100;

// -------- MQ2 adaptive thresholds --------
int   gasEMA = 0;            // filtered ADC counts (0..4095)
bool  gasAlarm = false;
int   gasBase = 0;           // moving baseline (ADC counts)
int   gasHigh_delta = 0;     // base + DELTA_HIGH or percent
int   gasLow_delta  = 0;

const int   DELTA_HIGH   = 60;        // smaller = more sensitive (counts)
const int   DELTA_LOW    = 30;        // clear below this (hysteresis)
const float PCT_HIGH     = 0.12f;     // +12% over baseline also trips
const float PCT_LOW      = 0.06f;     // clears when within +6%

const int   ABS_HIGH_MV  = 550;       // absolute trip (mV)
const int   ABS_CLEAR_MV = 420;       // absolute clear

// how fast baseline adapts when NO alarm (EMA style: 0..1)
const float BASE_ADAPT_ALPHA = 0.02f;  // slow drift tracking

// ---------- helpers ----------
void beep(int hz, int ms){ tone(BUZZER_PIN, hz, ms); delay(ms); noTone(BUZZER_PIN); }

long readDistanceCM(){
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (!us) return -1;
  return (us/2) * 0.0343;
}

bool canSend(unsigned long &lastTs){
  unsigned long now = millis();
  if (now - lastAny < CD_ANY_MS)   return false;
  if (now - lastTs  < CD_EVENT_MS) return false;
  lastTs = now; lastAny = now; return true;
}

void fireNTFY(const String& title, const String& body="", const String& pri="default"){
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http; http.begin(ntfyUrl());
  http.addHeader("Title", title);
  http.addHeader("Priority", pri);
  http.addHeader("Content-Type", "text/plain; charset=utf-8");
  int code = http.POST(body);
  Serial.printf("[NTFY] %s -> %d | %s\n", title.c_str(), code, body.c_str());
  http.end();
}

bool pressedNow(int pin){
  if (digitalRead(pin)==LOW){ delay(25); return digitalRead(pin)==LOW; }
  return false;
}

bool longPressForce(int pin, uint16_t ms=1000){
  if (digitalRead(pin)==LOW){
    unsigned long t0=millis();
    while (millis()-t0 < ms){ if (digitalRead(pin)!=LOW) return false; delay(5); }
    return true;
  }
  return false;
}

// ---------------- setup ----------------
void setup(){
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status()!=WL_CONNECTED){ delay(250); Serial.print("."); }
  Serial.println("\nWiFi connected.");

  pinMode(LED_TEMP, OUTPUT); pinMode(LED_GAS_A, OUTPUT);
  pinMode(LED_GAS_B, OUTPUT); pinMode(LED_DIST, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_OPEN, INPUT_PULLUP); pinMode(BTN_CLOSE, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);

  myServo.setPeriodHertz(50); myServo.attach(SERVO_PIN); myServo.write(0);
  sensors.begin();

  analogSetWidth(12);
  analogSetPinAttenuation(GAS_PIN, ADC_11db);

  // quick baseline
  Serial.println("Calibrating MQ-2 baseline (2s)...");
  unsigned long t0=millis(); uint32_t sum=0; uint16_t n=0;
  while (millis()-t0<2000){ int v=analogRead(GAS_PIN); sum+=v; n++; delay(10); }
  gasBase = gasEMA = (n? sum/n : 0);

  gasHigh_delta = gasBase + max(DELTA_HIGH, (int)(gasBase*PCT_HIGH));
  gasLow_delta  = gasBase + max(DELTA_LOW , (int)(gasBase*PCT_LOW));

  int mv = analogReadMilliVolts(GAS_PIN);
  Serial.printf("Base=%d (~%dmV) hiDelta=%d loDelta=%d | ABS_H=%dmV ABS_L=%dmV\n",
    gasBase, mv, gasHigh_delta, gasLow_delta, ABS_HIGH_MV, ABS_CLEAR_MV);

  Serial.println("Setup complete.");
}

// ---------------- loop ----------------
void loop(){
  // Temperature
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.printf("Temp: %.2f C\n", tempC);
  digitalWrite(LED_TEMP, (tempC!=DEVICE_DISCONNECTED_C && tempC>TEMP_C_THRESH) ? HIGH : LOW);
  if (tempC!=DEVICE_DISCONNECTED_C && tempC>TEMP_C_THRESH && canSend(lastTemp))
    fireNTFY("temp", String(tempC,2)+" C", "high");

  // GAS
  int gasRaw = analogRead(GAS_PIN);
  int gasmV  = analogReadMilliVolts(GAS_PIN);
  gasEMA = (gasEMA==0)? gasRaw : (gasEMA*9 + gasRaw)/10;           // α≈0.1

  // update baseline slowly ONLY when not in alarm
  if (!gasAlarm){
    gasBase = (int)((1.0f-BASE_ADAPT_ALPHA)*gasBase + BASE_ADAPT_ALPHA*gasEMA);
    gasHigh_delta = gasBase + max(DELTA_HIGH, (int)(gasBase*PCT_HIGH));
    gasLow_delta  = gasBase + max(DELTA_LOW , (int)(gasBase*PCT_LOW));
  }

  bool tripByDelta = (gasEMA >= gasHigh_delta);
  bool tripByAbs   = (gasmV  >= ABS_HIGH_MV);

  Serial.printf("MQ2 raw=%4d ema=%4d mv=%4d | base=%4d hi=%4d lo=%4d | trip[d=%d a=%d]\n",
    gasRaw, gasEMA, gasmV, gasBase, gasHigh_delta, gasLow_delta, tripByDelta, tripByAbs);

  if (!gasAlarm && (tripByDelta || tripByAbs)){
    gasAlarm = true;
    for (int i=0;i<3;i++){
      digitalWrite(LED_GAS_A,HIGH); digitalWrite(LED_GAS_B,LOW);  beep(1000,120);
      digitalWrite(LED_GAS_A,LOW);  digitalWrite(LED_GAS_B,HIGH); beep(1250,120);
    }
    myServo.write(90);
    if (canSend(lastGas)){
      String body = "EMA="+String(gasEMA)+" raw="+String(gasRaw)+
                    " mV="+String(gasmV)+" base="+String(gasBase)+
                    " trip="+String(tripByDelta? "delta":"abs");
      fireNTFY("gas_alarm", body, "max");
    }
  } else if (gasAlarm && (gasEMA <= gasLow_delta && gasmV <= ABS_CLEAR_MV)){
    gasAlarm = false;
    digitalWrite(LED_GAS_A,LOW); digitalWrite(LED_GAS_B,LOW);
    myServo.write(0);
    if (canSend(lastGas)){
      String body = "cleared: ema="+String(gasEMA)+" mV="+String(gasmV);
      fireNTFY("gas_clear", body, "default");
    }
  }

  // Force test (hold OPEN ≥1s)
  if (longPressForce(BTN_OPEN)) {
    Serial.println("FORCE TEST: gas alarm");
    gasAlarm = false; gasEMA = gasHigh_delta + 1; // will trip on next iteration
  }

  // Manual door
  if (pressedNow(BTN_OPEN)){ myServo.write(90); beep(1400,80); if (canSend(lastDoor)) fireNTFY("door_opened"); }
  if (pressedNow(BTN_CLOSE)){ myServo.write(0);  beep(800,80);  if (canSend(lastDoor)) fireNTFY("door_closed"); }

  // Ultrasonic
  long d = readDistanceCM();
  if (d<0 || d<2 || d>315){ digitalWrite(LED_DIST,LOW); Serial.println("Distance: invalid"); }
  else if (d<=DIST_NEAR_MAX){ digitalWrite(LED_DIST,HIGH); Serial.printf("Distance: %ld cm (near)\n", d);
    if (canSend(lastDist)) fireNTFY("distance", String(d)+" cm", "high"); }
  else { digitalWrite(LED_DIST,LOW); Serial.printf("Distance: %ld cm\n", d); }

  Serial.println("------------------------------");
  delay(250);
}
