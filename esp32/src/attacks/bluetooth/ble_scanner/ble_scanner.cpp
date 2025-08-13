#include "ble_scanner.h"
#include <NimBLEDevice.h>

std::vector<String> bleNameList;
std::vector<String> bleMacList;
int bleCount = 0;

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
        String name = advertisedDevice->getName().c_str();
        String mac = advertisedDevice->getAddress().toString().c_str();

        if (name.length() == 0) {
            name = "Unknown";
        }

        bleNameList.push_back(name);
        bleMacList.push_back(mac);
        bleCount = bleNameList.size();
    }
};

void scanBLEDevices() {
    bleNameList.clear();
    bleMacList.clear();
    bleCount = 0;

    NimBLEDevice::init("");
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
    pScan->setActiveScan(true);
    pScan->start(5, false); // scan for 5 seconds
}

