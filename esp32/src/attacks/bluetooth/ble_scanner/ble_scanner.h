#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H
#include <vector>
#include <Arduino.h>

extern std::vector<String> bleNameList;  // BLE device names
extern std::vector<String> bleMacList;   // BLE MAC addresses


void scanBLEDevices();

#endif
