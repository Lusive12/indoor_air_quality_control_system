# DesIoT: Automated Indoor Air Quality Management System

![Project Banner/Hardware Image](link-to-your-image-in-docs-folder.jpg)

## 📌 Overview
DesIoT is a complete, edge-to-cloud IoT solution designed to actively monitor and manage indoor air quality. Rather than just acting as a passive sensor node, this system implements edge automation to control ventilation and humidity, while simultaneously logging time-series data to the cloud for further analysis.

## ⚙️ System Architecture
The architecture is divided into an Edge Controller (ESP32) and a Data Gateway (Raspberry Pi 4):

1. **Sensing:** The ESP32 gathers environmental data via I2C from the **ENS160** (eCO2, TVOC, AQI) and **AHT21** (Temperature, Humidity).
2. **Edge Automation:** The ESP32 evaluates the data against predefined thresholds to automatically trigger a 12V ventilation fan or a 5V humidifier via a 2-channel relay.
3. **Serial JSON Bridge:** To ensure high reliability, the ESP32 packages the raw sensor data into a clean JSON string and transmits it over a wired USB Serial connection to the Raspberry Pi at a 115200 baud rate.
4. **Cloud Gateway:** A Python script on the Raspberry Pi auto-detects the ESP32, parses the incoming JSON, and pushes the time-series data to **ThingSpeak** via HTTP GET requests.

## 🧰 Hardware Components
* **Microcontroller:** ESP32 DevKit V4
* **Gateway:** Raspberry Pi 4 Model B
* **Air Quality Sensor:** ScioSense ENS160 (MOx Sensor)
* **Climate Sensor:** Adafruit AHT21
* **Display:** LCD 16x2 with I2C Backpack
* **Actuators:** 2-Channel Relay Module (Active LOW), 12V DC Fan, 5V Humidifier Module

## 🔌 Wiring & Pinout
| Component | ESP32 Pin | Note |
| :--- | :--- | :--- |
| **ENS160 & AHT21** | GPIO 21 (SDA), GPIO 22 (SCL) | I2C Bus |
| **LCD 16x2** | GPIO 21 (SDA), GPIO 22 (SCL) | I2C Bus (Address `0x27`) |
| **Fan Relay (IN1)** | GPIO 12 | Triggers when TVOC > 150ppb or eCO2 > 1000ppm |
| **Humidifier Relay (IN2)** | GPIO 32 | Triggers when Humidity < 40% |

## 🚀 Quick Start
### 1. ESP32 Setup
* Flash the `main.cpp` code located in the `/firmware` directory using PlatformIO or Arduino IDE.
* Required Libraries: `ScioSense ENS16x`, `Adafruit AHTX0`, `Arduino_JSON`, `LiquidCrystal_I2C`.

### 2. Raspberry Pi Setup
* Install required Python dependencies:
  ```bash
  pip install pyserial requests