#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <Arduino.h>

// Initializes app control mode (before main menu)
void initAppControlMode();

// Checks and sets the control mode (button vs app)
void selectControlMode();

// Returns true if app is controlling the ESP32
bool isAppControlActive();
void handleRestAPI();

#endif
