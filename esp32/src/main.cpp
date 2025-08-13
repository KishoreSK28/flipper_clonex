#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "globals.h"
#include "control_mode\control_mode.h"
#include "storage/server.h"
#include "app_control/app_control.h"


// Custom modules
#include "ui.h"
#include "attacks/wifi/eviltwin.h"

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    delay(200);
    initUI();         // OLED, buttons, menu
}

void loop() {
    updateUI();       // Handle menu, button inputs
    handleDNS();      // DNS hijacking for Evil Twin
    server.handleClient();
}
