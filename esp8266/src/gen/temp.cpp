#include <Arduino.h>
#include "temp.h"
#include <DHT.h>

// Pin setup
#define DHTPIN D7        // Sensor connected to D7 (GPIO13)
#define DHTTYPE DHT11    // Change to DHT22 if you have that sensor
const int fanPin = D1;   // Control pin for transistor base

DHT dht(DHTPIN, DHTTYPE);

// Temperature threshold in °C
const float TEMP_THRESHOLD = 30.0;  

void initTemp() {
    dht.begin();
    pinMode(fanPin, OUTPUT);
    digitalWrite(fanPin, LOW); // Fan OFF initially
}

float readTemp() {
    float t = dht.readTemperature();
    if (isnan(t)) {
        Serial.println("[TEMP] Error reading sensor!");
        return -100.0;  // Invalid reading
    }
    return t;
}

void controlFan() {
    float temp = readTemp();
    if (temp >= TEMP_THRESHOLD) {
        digitalWrite(fanPin, HIGH);  // Fan ON
        Serial.printf("[TEMP] %.2f °C → Fan ON\n", temp);
    } else {
        digitalWrite(fanPin, LOW);   // Fan OFF
        Serial.printf("[TEMP] %.2f °C → Fan OFF\n", temp);
    }
}
