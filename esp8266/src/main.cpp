#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <user_interface.h>
#include "attack/deauth.h"
#include "attack/beacon.h"
#include <gen/temp.h>


// Definition of RxControl struct as required by ESP8266 SDK
struct RxControl
{
    signed rssi : 8;
    unsigned rate : 4;
    unsigned is_group : 1;
    unsigned : 1;
    unsigned sig_mode : 2;
    unsigned legacy_length : 12;
    unsigned damatch0 : 1;
    unsigned damatch1 : 1;
    unsigned bssidmatch0 : 1;
    unsigned bssidmatch1 : 1;
    unsigned MCS : 7;
    unsigned CWB : 1;
    unsigned HT_length : 16;
    unsigned Smoothing : 1;
    unsigned Not_Sounding : 1;
    unsigned : 1;
    unsigned Aggregation : 1;
    unsigned STBC : 2;
    unsigned FEC_CODING : 1;
    unsigned SGI : 1;
    unsigned rxend_state : 8;
    unsigned ampdu_cnt : 8;
    unsigned channel : 4;
    unsigned : 12;
};

// Store up to 10 unique clients
struct MacRecord
{
    uint8_t client[6];
    uint8_t bssid[6];
};

MacRecord foundClients[10];
int foundCount = 0;

// Promiscuous sniff callback
void snifferCallback(uint8_t *buf, uint16_t len)
{
    if (len == 0)
        return;

    uint8_t *data = buf + sizeof(struct RxControl);

    // Check for data frame (0x08 subtype)
    if ((data[0] & 0x0C) == 0x08 && len > 36)
    {
        uint8_t *src = data + 10;   // Client MAC
        uint8_t *bssid = data + 16; // AP MAC

        // Check if client is already seen
        bool seen = false;
        for (int i = 0; i < foundCount; i++)
        {
            if (memcmp(foundClients[i].client, src, 6) == 0)
            {
                seen = true;
                break;
            }
        }

        if (!seen && foundCount < 10)
        {
            memcpy(foundClients[foundCount].client, src, 6);
            memcpy(foundClients[foundCount].bssid, bssid, 6);
            Serial.printf("[ESP8266] Found client %02X:%02X:%02X:%02X:%02X:%02X\n",
                          src[0], src[1], src[2], src[3], src[4], src[5]);
            foundCount++;
        }
    }
}

void setup()
{
    Serial.begin(9600);
    initTemp();
    delay(1000);

    WiFi.mode(WIFI_OFF); // Ensure WiFi stack is reset
    delay(100);
    wifi_set_opmode(STATION_MODE); // Enter station mode
    wifi_promiscuous_enable(0);    // Disable first, just to be safe
    delay(10);
    wifi_set_promiscuous_rx_cb(snifferCallback); // Set sniffer
    wifi_promiscuous_enable(1);                  // Enable monitor mode

    Serial.println("[ESP8266] Monitor mode enabled. Sniffing...");
}

void loop()
{
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.startsWith("START_DEAUTH_BROADCAST"))
        {
            int space1 = input.indexOf(' ');
            int space2 = input.indexOf(' ', space1 + 1);

            if (space1 > 0 && space2 > space1)
            {
                String bssidStr = input.substring(space1 + 1, space2);
                String chStr = input.substring(space2 + 1);
                uint8_t bssid[6];
                int channel = chStr.toInt();

                if (sscanf(bssidStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                           &bssid[0], &bssid[1], &bssid[2],
                           &bssid[3], &bssid[4], &bssid[5]) == 6)
                {

                    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    startDeauth(bssid, broadcast, channel);
                    Serial.println("[ESP8266] Broadcast deauth started.");
                }
                else
                {
                    Serial.println("[ESP8266] Invalid BSSID format");
                }
            }
            else
            {
                Serial.println("[ESP8266] Invalid START_DEAUTH_BROADCAST syntax");
            }
        }

        else if (input == "STOP_DEAUTH")
        {
            stopDeauth();
            Serial.println("[ESP8266] Deauth stopped");
        }
        else if(input.startsWith("START_BEACON_FLOOD")){
            // startbeaconflood();
            sendsinglebeacon();
        }
        else if (input = "STOP_BEACON")
        {
            stopbeaconflood();
        }
        

        else
        {
            Serial.println("[ESP8266] Unknown command");
        }
    }
    controlFan();
    delay(100);
}
