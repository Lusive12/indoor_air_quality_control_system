/*
 * DesIoT: Automated Indoor Air Quality System
 * ESP32 + ENS160 + AHT21 + LCD 16x2 + 2-Channel Relay
 * Library: ScioSense ENS16x, Adafruit AHTX0, Arduino_JSON
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ENS160.h" 
#include "Adafruit_AHTX0.h"
#include <Arduino_JSON.h>

// Pin Configuration
const int RELAY_FAN_PIN = 12;        
const int RELAY_HUMIDIFIER_PIN = 32; 

// Actuator Thresholds
const int TVOC_THRESHOLD_HIGH = 150;       
const int CO2_THRESHOLD_HIGH = 1000;       
const float HUMIDITY_THRESHOLD_LOW = 40.0; 

// Object Initialization
const int LCD_ADDRESS = 0x27;
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
ENS160 ens160; 
Adafruit_AHTX0 aht;

// Global Variables
float currentTemperature = 0.00;
float currentHumidity = 0.00;
uint16_t currentECO2 = 0;
uint16_t currentTVOC = 0;
uint8_t currentAQI = 0;

unsigned long lastReadingTime = 0;
const unsigned long readingInterval = 2000; 
bool showAirQuality = true;
unsigned long lastDisplaySwitchTime = 0;
const long displayInterval = 5000; 

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_FAN_PIN, OUTPUT);
  pinMode(RELAY_HUMIDIFIER_PIN, OUTPUT);
  digitalWrite(RELAY_FAN_PIN, HIGH); 
  digitalWrite(RELAY_HUMIDIFIER_PIN, HIGH);

  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.print("System Init...");
  
  if (!aht.begin()) {
    lcd.setCursor(0, 1); lcd.print("AHT Error");
    while (1) delay(10);
  }

  if (!ens160.begin(Wire, 0x53)) { 
    lcd.setCursor(0, 1); lcd.print("ENS160 Error");
    while (1) delay(10);
  }
  
  ens160.setMode(ENS160_OPMODE_STD);
  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastReadingTime >= readingInterval) {
    lastReadingTime = currentTime;
    readSensors();      
    controlActuators(); 
    sendDataSerial();   
    updateDisplay();    
  }

  if (currentTime - lastDisplaySwitchTime >= displayInterval) {
    showAirQuality = !showAirQuality;
    lastDisplaySwitchTime = currentTime;
    updateDisplay(); 
  }
}

void readSensors() {
  sensors_event_t humidity_event, temp_event;
  aht.getEvent(&humidity_event, &temp_event);
  
  currentTemperature = temp_event.temperature;
  currentHumidity = humidity_event.relative_humidity;

  ens160.set_envdata(currentTemperature, currentHumidity);
  ens160.measure(true); 

  if (ens160.available()) {
    currentAQI = ens160.get_aqi();
    currentTVOC = ens160.get_tvoc();
    currentECO2 = ens160.get_eco2();
  }
}

void controlActuators() {
  if (currentTVOC == 0 && currentECO2 == 400) return;

  if (currentTVOC > TVOC_THRESHOLD_HIGH || currentECO2 > CO2_THRESHOLD_HIGH) {
    digitalWrite(RELAY_FAN_PIN, LOW); 
  } else {
    digitalWrite(RELAY_FAN_PIN, HIGH); 
  }

  if (currentHumidity < HUMIDITY_THRESHOLD_LOW) {
    digitalWrite(RELAY_HUMIDIFIER_PIN, LOW); 
  } else {
    digitalWrite(RELAY_HUMIDIFIER_PIN, HIGH); 
  }
}

void sendDataSerial() {
  if (currentTVOC == 0 && currentECO2 == 400) return; 

  JSONVar data;
  data["temperature"] = currentTemperature;
  data["humidity"] = currentHumidity;
  data["eco2"] = currentECO2;
  data["tvoc"] = currentTVOC;
  data["aqi"] = currentAQI;

  String jsonString = JSON.stringify(data);
  Serial.println(jsonString);
}

void updateDisplay() {
  if (currentTVOC == 0 && currentECO2 == 400) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Sensor WarmingUp");
    lcd.setCursor(0, 1); lcd.print("Please Wait...");
    return;
  }

  lcd.clear();
  if (showAirQuality) {
    lcd.setCursor(0, 0);
    lcd.print("AQI:" + String(currentAQI));
    lcd.setCursor(6, 0); 
    lcd.print("TVOC:" + String(currentTVOC));
    lcd.setCursor(0, 1);
    lcd.print("eCO2:" + String(currentECO2) + "ppm");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + String(currentTemperature, 1) + (char)223 + "C");
    lcd.setCursor(0, 1);
    lcd.print("Humi: " + String(currentHumidity, 1) + "%");
  }
}