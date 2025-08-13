#ifndef DEAUTH_H
#define DEAUTH_H
#pragma once
#include <Arduino.h>

void startDeauth(uint8_t* bssid, int channel);
void stopDeauth();

extern volatile unsigned long deauthPacketCount;
extern String targetSSID;

#endif // DEAUTH_H
