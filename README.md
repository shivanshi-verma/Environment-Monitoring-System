# 🧠 Environment Monitoring System (EMS)

An **IoT-based Environment Monitoring System** using **ESP32** that detects **temperature, gas concentration, and object proximity** in real time.
It provides **local alerts** (LEDs, buzzer, servo) and **remote cloud notifications** via **ntfy.sh** using Wi-Fi and HTTP.

---

## 🌟 Features

* Multi-sensor integration: DS18B20, MQ-2, HC-SR04
* Adaptive baseline tracking and EMA filtering
* Real-time cloud alerts via ntfy.sh
* Manual servo control and force test mode
* Works both on **Wokwi simulator** and **ESP32 hardware**

---

## 🧰 Hardware & Interfaces

| Component         | Function                                       |
| ----------------- | ---------------------------------------------- |
| **ESP32**         | Main microcontroller (Wi-Fi + processing)      |
| **DS18B20**       | Temperature sensor (OneWire interface)         |
| **MQ-2**          | Gas/smoke detection (Analog input)             |
| **HC-SR04**       | Distance/proximity sensing (Digital TRIG/ECHO) |
| **Servo Motor**   | Door/valve actuation                           |
| **Buzzer & LEDs** | Local audio-visual alert indicators            |
| **Wi-Fi & HTTP**  | Cloud communication via ntfy.sh                |

---

## 🚀 How It Works

1. ESP32 reads temperature, gas, and distance data.
2. MQ-2 data filtered via **EMA** and adaptive baseline calibration.
3. If unsafe condition detected:

   * Servo activates
   * Buzzer and LEDs trigger
   * Cloud alert sent to **ntfy.sh topic**
4. Alerts received instantly on mobile or web browser.

---

## ☁️ Cloud Notification Setup (ntfy.sh)

1. Open [https://ntfy.sh/sensor_project_alerts](https://ntfy.sh/sensor_project_alerts).
2. Subscribe via browser or mobile ntfy app.
3. Upload the `.ino` file to ESP32 and connect Wi-Fi.
4. Receive real-time alerts (temperature, gas, distance, door).

---

## 🧪 Simulation and Testing

* Simulated using **Wokwi ESP32 environment**.
* Validated on physical ESP32 hardware.
* Average alert latency: **1–2 seconds**.

---

## 📸 Screenshots
Figure 1: Wokwi Simulation of Environment Monitoring System
<img width="775" height="459" alt="image" src="https://github.com/user-attachments/assets/419f1176-2273-47e9-a8d3-b932d7e27939" />
The circuit diagram depicts the complete hardware connections associated with the Environment Monitoring System (EMS) incorporated onto the ESP32 microcontroller. The complete hardware connections included various sensors — including the MQ-2 for gas detection, the DS18B20 for temperature, and the HC-SR04 for distance. The complete hardware connections also incorporated a servo motor, a buzzer, and various light-emitting diode indicators. The circuit wiring appropriately interfaced the signals of analog, digital, and PWM (Pulse-width modulation) signals, while the ESP32 microcontroller managed all sensing, actuation, and Wi-Fi communication related to cloud-based notifications (ntfy.sh).

Figure 2: Serial Monitor Output of Environment Monitoring System
<img width="783" height="613" alt="image" src="https://github.com/user-attachments/assets/94694fe7-ee93-4f6d-a391-69e1a2a1ab15" />
The serial monitor output displays the initialization and ongoing operation of the Environment Monitoring System (EMS). It demonstrates that the Wi-Fi connection was successful, the MQ-2 sensor baseline was calibrated, and it shows the live sensor readings for temperature, gas concentration, and distance. The results demonstrated accurate acquisition of environmental data, and that the system is in a state of readiness for adaptive real-time monitoring with cloud-based alerting.

Figure 4.3: Real-Time Serial Monitor Output During System Operation
<img width="940" height="669" alt="image" src="https://github.com/user-attachments/assets/8ce168d2-db5d-4db2-88e8-e68735153eb0" />
The serial monitor output displays the real-time operation of the Environment Monitoring System (EMS). It displays live values from the DS18B20 temperature sensor, the MQ-2 gas sensor, and the HC-SR04 ultrasonic sensor. When the temperature and distance limits were reached, the system automatically triggered the appropriate alerts and notified the ntfy.sh cloud service. The FORCE TEST command simulated a manual test of the gas alarm condition, which enabled validation of real-time operation, accuracy of the system, adaptive calibration, and alerting mechanisms.

Figure 4.3: Cloud Notification Alerts from ntfy.sh
<img width="557" height="1238" alt="image" src="https://github.com/user-attachments/assets/338490ca-64a0-436a-a349-4b6437f47d15" />
The figure illustrates real-time cloud notifications produced by the Environment Monitoring System (EMS) through the ntfy.sh service. Notifications included alerts for temperature, gas, distance, and door status. Notification status was sent in the form of HTTP POST requests from the ESP32 microcontroller. Each notification represents an event that was detected (e.g., a temperature increase from a value above 39°C, proximity detection, and gas detection). Notifications were sent to the user's mobile device with real-time notifications, validating IoT deployment of reliable and real-time communication and awareness for hazards or hazards developing remotely.

Figure 4.4: LED Indicator Response During Alert Condition
<img width="557" height="1238" alt="image" src="https://github.com/user-attachments/assets/d11821ce-2547-4bca-96e5-94c3d14bcc20" />
The figure indicates the LED status of the Environment Monitoring System (EMS) upon real-time operation. The yellow LED indicates a proximity alert detected when the HC-SR04 ultrasonic sensor detected an object within 100 cm. The white LED indicates a temperature alert activated when the DS18B20 sensor detected a temperature above 39°C. The green LED continuously produces 110V to indicate that the system is receiving power and is actively monitoring environmental conditions. The other LEDs indicate the system does not sense conditions associated with gas or other hazards. This illustrates that the LEDs are functioning correctly, and ESP32 is managing the LEDs for accurate visual feedback based upon events.


## ⚙️ How to Run

1. Open **Environment_Monitoring_System.ino** in Arduino IDE.
2. Select board → **ESP32 Dev Module**.
3. Install required libraries:

   * `WiFi.h`
   * `HTTPClient.h`
   * `ESP32Servo.h`
   * `DallasTemperature.h`
   * `OneWire.h`
4. Set Wi-Fi credentials.
5. Upload to ESP32 or simulate on **Wokwi**.

---

## 📈 Future Enhancements

* Add CO₂ and humidity sensors.
* Connect to cloud dashboard (Firebase / ThingSpeak).
* Integrate AI-based hazard prediction.
* Mobile app and voice assistant support.
* Automated ventilation control.

---

## 🧑‍💻 Author

**Shivanshi Verma (22BCT0103)**
[LinkedIn Profile](https://www.linkedin.com/in/shivanshi-verma-99b299257?utm_source=share&utm_campaign=share_via&utm_content=profile&utm_medium=android_app)

---

## 📄 License

MIT License © 2025 Shivanshi Verma

---

