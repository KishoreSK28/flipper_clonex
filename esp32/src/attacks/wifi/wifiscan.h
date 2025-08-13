#pragma once

#include <Arduino.h>

extern String ssidList[20];
extern uint8_t bssidList[20][6];
extern int channelList[20];
extern int apCount;

void scanWiFiNetworks();
