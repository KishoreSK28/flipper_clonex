#ifndef BEACON_H
#define BEACON_H

#include <Arduino.h>
#include<ESP8266WiFi.h>
#include<Ticker.h>

void startbeaconflood();

void stopbeaconflood();

void sendsinglebeacon();



#endif BEACON_H