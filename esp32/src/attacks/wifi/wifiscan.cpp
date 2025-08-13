#include "wifiscan.h"
#include <WiFi.h>

String ssidList[20];
uint8_t bssidList[20][6];
int channelList[20];
int apCount = 0;

void scanWiFiNetworks() {
    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect(true); // Clear previous connections
    delay(200);

    apCount = WiFi.scanNetworks(false, true); // async=false, showHidden=true

    // Store SSID, BSSID, Channel
    for (int i = 0; i < apCount && i < 20; ++i) {
        ssidList[i] = WiFi.SSID(i);
        memcpy(bssidList[i], WiFi.BSSID(i), 6);
        channelList[i] = WiFi.channel(i); // ✅ Store channel here
    }

    Serial.println("[WiFi Scan] Completed:");
    for (int i = 0; i < apCount && i < 20; ++i) {
        Serial.print(i);
        Serial.print(": ");
        Serial.print(ssidList[i]);
        Serial.print(" [");

        // Print BSSID
        for (int j = 0; j < 6; ++j) {
            if (j > 0) Serial.print(":");
            Serial.print(bssidList[i][j], HEX);
        }

        Serial.print("] channel: ");
        Serial.println(channelList[i]);
    }
}
