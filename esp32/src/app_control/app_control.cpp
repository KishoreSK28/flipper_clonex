#include "./attacks/wifi/eviltwin.h"
#include "./attacks/wifi/deauth.h"
#include "./globals.h"
#include "./attacks/bluetooth/ble_scanner/ble_scanner.h"
#include "./attacks/bluetooth/gatt_server/gatt_server.h"
#include "./attacks/bluetooth/ble_spoof/ble_spoof.h"
#include <WebServer.h>
#include <WiFi.h>
#include "./storage/server.h"


void scanwifinetworks();
void handleRestAPI();
void handleRoot();
void handleWiFiScan();
void handleEvilTwinStart();
void handleDeauthStart();
void stopAttacks();
void handleStorage();
void blescan();
void gattserver();
void startBLESpoof();



void initAppControlMode()
{
    WiFi.softAP("Flipper Clonex", "123123123");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("ap IP ADDRESS:");
    Serial.print(IP);

    handleRestAPI();
    server.begin();
}

void handleRoot()
{
    server.send(200, "text/plain", "FlipperCloneX API Online");
}

void handleWiFiScan()
{
    scanwifinetworks(); // Your existing function
    server.send(200, "application/json", "{\"status\": \"wifi_scan_started\"}");
}

void handleEvilTwinStart()
{
    startEvilTwin(); // Call your function
    server.send(200, "application/json", "{\"status\": \"evil_twin_started\"}");
}

void handleDeauthStart()
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "application/json", "{\"error\":\"Missing JSON\"}");
        return;
    }

    String body = server.arg("plain");
    // Expected format: {"bssid":"AA:BB:CC:DD:EE:FF","channel":6}

    int bssidStart = body.indexOf("\"bssid\":\"") + 9;
    int bssidEnd = body.indexOf("\"", bssidStart);
    String bssidStr = body.substring(bssidStart, bssidEnd);

    int channelStart = body.indexOf("\"channel\":") + 10;
    int channelEnd = body.indexOf("}", channelStart);
    int channel = body.substring(channelStart, channelEnd).toInt();

    // Convert bssid string to byte array
    uint8_t bssid[6];
    sscanf(bssidStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &bssid[0], &bssid[1], &bssid[2],
           &bssid[3], &bssid[4], &bssid[5]);

    Serial2.print("START_DEAUTH_BROADCAST"); // Add the actual function name
    server.send(200, "application/json", "{\"status\": \"deauth_started\"}");
}

// ------------- Register Routes -------------
void stopattacks(){
    Serial2.println("STOP_DEAUTH");

}

void handlestorage(){
    startStorageServer(server);
}
void blescan() {
    scanBLEDevices();
    String json = "[";
    for (size_t i = 0; i < bleNameList.size(); ++i) {
        if (i > 0) json += ",";
        json += "{\"name\":\"" + bleNameList[i] + "\",\"mac\":\"" + bleMacList[i] + "\"}";
    }
    json += "]";
    server.send(200, "application/json", json);

}
void gattserver() {
    startFakeGATTServer();
    server.send(200, "application/json", "{\"status\": \"gatt_server_started\"}");
}
void startBLESpoof() {
    startBLESpoof();// Implement your BLE spoofing logic here
    server.send(200, "application/json", "{\"status\": \"ble_spoof_started\"}");
}

void handleRestAPI()
{
    server.on("/", handleRoot);
    server.on("/wifi/scan", handleWiFiScan);
    server.on("/wifi/eviltwin", handleEvilTwinStart);
    server.on("/wifi/deauth", handleDeauthStart);
    server.on("/wifi/stop" , stopattacks);
    server.on("/storage" , handlestorage);
    server.on("/bluetooth/scan", blescan);
    server.on("/bluetooth/gatt", gattserver);
    server.on("/bluetooth/spoof" , startBLESpoof);
}

void scanwifinetworks()
{
    int n = WiFi.scanNetworks();
    String json = "[";

    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"bssid\":\"" + WiFi.BSSIDstr(i) + "\",";
        json += "\"channel\":" + String(WiFi.channel(i));
        json += "}";
    }

    json += "]";
    server.send(200, "application/json", json);
}