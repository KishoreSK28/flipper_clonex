#ifndef EVILTWIN_H
#define EVILTWIN_H
#include <Arduino.h>

void initEvilTwin();
void startEvilTwin();
void processEvilTwinDNS();
void handleDNS();
void handleGetCredentials();
// Declare these as extern

extern String ssid;
extern String pass;
extern bool capnew;


#endif
