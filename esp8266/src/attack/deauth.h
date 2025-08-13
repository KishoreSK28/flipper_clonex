#ifndef DEAUTH_H
#define DEAUTH_H

#include <Arduino.h>
#include <Ticker.h>

extern Ticker deauthTicker;
extern uint8_t deauthPacket[26];
extern int deauthPacketCount;

void sendDeauthTick();
void startDeauth(uint8_t *ap, uint8_t *client, int channel);
void stopDeauth();

#endif
