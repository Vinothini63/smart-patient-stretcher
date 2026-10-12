# 🚀 ESP32 Smart Wheelchair – Complete Edition

## 📌 Project Overview

The **ESP32 Smart Wheelchair** is an embedded systems and IoT-based project designed to provide convenient wheelchair movement and basic safety monitoring. It uses an ESP32 microcontroller to control motor movement through Bluetooth voice commands and a Wi-Fi-based web dashboard.

The system integrates GPS tracking to obtain location information and an IR sensor to detect obstacles and stop motor movement when an obstacle is detected. A buzzer provides an alert, while the web dashboard displays GPS information, motor speed settings, and obstacle status.

## ✨ Features

- 🎮 **Motor Control:** Forward, backward, left, right, and stop operations.
- 🎙️ **Bluetooth Control:** Accepts text commands from a Bluetooth terminal or a compatible voice-control application.
- 🌐 **Web Dashboard:** Browser-based controls for wheelchair movement and speed adjustment.
- 📍 **GPS Tracking:** Retrieves latitude, longitude, speed, and satellite information.
- 🛡️ **Obstacle Detection:** Uses an IR sensor to detect obstacles and stop motor movement.
- 🚨 **Emergency Stop:** Provides a stop command through Bluetooth and the web dashboard.
- 🔔 **Buzzer Alert:** Provides audible alerts and supports buzzer testing.
- ⚙️ **Adjustable Motor Speed:** Supports PWM-based motor speed control.
- 📊 **Live Status Monitoring:** Displays GPS data, motor speed settings, and obstacle status on the dashboard.

## 🧰 Hardware Requirements

- ESP32 development board
- Two DC geared motors suitable for the wheelchair prototype
- Compatible dual-channel motor driver
- IR obstacle detection sensor
- GPS module (such as NEO-6M)
- Active or compatible buzzer
- Rechargeable battery and suitable power regulation
- Wheelchair prototype chassis and wheels
- Jumper wires and connecting accessories

## 💻 Software Requirements

- Arduino IDE
- ESP32 board package
- ESP32-compatible BluetoothSerial library
- Wi-Fi and WebServer libraries
- TinyGPSPlus library
- Web browser
- Bluetooth terminal or compatible voice-control application

## 🔌 Pin Configuration

| Component | ESP32 Pin |
|---|---|
| Motor Input IN1 | GPIO 26 |
| Motor Input IN2 | GPIO 27 |
| Motor Input IN3 | GPIO 14 |
| Motor Input IN4 | GPIO 12 |
| Motor Enable ENA | GPIO 25 |
| Motor Enable ENB | GPIO 33 |
| IR Sensor | GPIO 13 |
| Buzzer | GPIO 5 |
| GPS RX | GPIO 22 |
| GPS TX | GPIO 23 |

**Note:** Connect the GPS module's TX output to ESP32 GPIO 22 (GPS RX), and its RX input to ESP32 GPIO 23 (GPS TX). Verify the motor driver's enable-pin requirements and ESP32 pin compatibility before powering the system.

## ⚙️ Working Principle

1. The ESP32 initializes the motor control pins, GPS module, Bluetooth interface, and Wi-Fi connection.
2. The motor driver receives control signals from the ESP32 to operate the motors in the requested direction.
3. Bluetooth commands or web dashboard controls select the movement direction and motor speed.
4. The GPS module supplies location and speed information for monitoring.
5. The IR sensor continuously checks for obstacles.
6. When the sensor detects an obstacle, the program stops motor drive signals and activates the buzzer.
7. The web dashboard periodically requests system status and displays the available GPS and obstacle information.

## 🎙️ Bluetooth Commands

| Command | Function |
|---|---|
| `F` / `FORWARD` / `GO` | Move forward |
| `B` / `BACKWARD` / `BACK` | Move backward |
| `L` / `LEFT` | Turn left |
| `R` / `RIGHT` | Turn right |
| `S` / `STOP` / `HALT` | Stop movement |
| `FASTER` | Increase speed |
| `SLOWER` | Decrease speed |
| `WHERE` / `LOCATION` / `GPS` | Request GPS location |
| `STATUS` | Display system status |
| `HELP` | Display supported commands |

Bluetooth commands must be sent as newline-terminated text. Voice control requires a compatible application that converts speech into these commands.

## 🌐 Web Dashboard

The web dashboard provides:

- Directional movement buttons
- Motor speed adjustment slider
- GPS location display
- Embedded map view
- Satellite and GPS speed information
- Obstacle warning display
- Emergency stop control
- Buzzer test control

### How to Access the Dashboard

1. Configure your Wi-Fi credentials in the Arduino sketch.
2. Upload the program to the ESP32.
3. Open the Serial Monitor at **115200 baud**.
4. Wait for the ESP32 to connect to Wi-Fi.
5. Note the IP address printed in the Serial Monitor.
6. Open that IP address in a browser on a device connected to the same network.

## 📚 Libraries Used

- `WiFi.h`
- `WebServer.h`
- `TinyGPSPlus.h`
- `BluetoothSerial.h`

The Wi-Fi, web server, and BluetoothSerial libraries are included with the appropriate ESP32 Arduino core. Install TinyGPSPlus separately if it is not already available.

## 🔬 Applications

- Embedded systems learning
- IoT-based mobility prototypes
- Bluetooth-controlled robotic vehicles
- GPS-enabled mobile systems
- Obstacle detection and motor control demonstrations
- Assistive technology research and development

## 🚀 Future Enhancements

- Integration of ultrasonic or additional obstacle sensors
- Improved braking and emergency-stop hardware
- Caregiver monitoring and remote alerts
- GPS location sharing through a secure remote service
- Improved voice recognition
- Battery-level monitoring
- More robust fault detection and recovery

## ⚠️ Safety Notice

This project is an educational prototype and is **not validated for transporting a person**. The current software requires additional safety engineering and testing before any human-use application. Obstacle detection depends on sensor coverage, and the current emergency-stop implementation is software-based. Use a suitable independent hardware emergency stop, a fail-safe braking system, current protection, and a mechanically stable chassis. Test the system without a passenger and with the drive wheels safely raised before conducting controlled ground tests.

## 👩‍💻 Project Type

Embedded Systems | Internet of Things (IoT) | ESP32 | Bluetooth Control | GPS Tracking | Motor Control | Obstacle Detection

---

**If you find this project useful, consider starring the repository!**
