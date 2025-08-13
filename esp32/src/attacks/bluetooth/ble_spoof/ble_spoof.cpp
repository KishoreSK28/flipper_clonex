#include <NimBLEDevice.h>
#include "BLE_Spoof.h"

void startBLESpoof() {
  NimBLEDevice::init("ESP32_Spoofed_Device");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Max TX power

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();

  advertising->addServiceUUID("abcd");         // Spoofed UUID
  advertising->setName("Fake_Tracker");        // Fake name shown to scanners
  advertising->setAppearance(0x1234);          // Optional: Set fake device type
  advertising->start();

  Serial.println("[BLE SPOOF] Advertising fake device started.");
}
