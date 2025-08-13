#include <NimBLEDevice.h>
#include "GATT_Server.h"

#define FAKE_SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define FAKE_CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0"

void startFakeGATTServer() {
  NimBLEDevice::init("Fake_GATT_Server");
  NimBLEServer* server = NimBLEDevice::createServer();
  NimBLEService* service = server->createService(FAKE_SERVICE_UUID);

  NimBLECharacteristic* characteristic = service->createCharacteristic(
    FAKE_CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  );

  characteristic->setValue("Hello from fake server");
  service->start();

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(FAKE_SERVICE_UUID);
  advertising->start();

  Serial.println("[GATT] Fake GATT server started.");
}
