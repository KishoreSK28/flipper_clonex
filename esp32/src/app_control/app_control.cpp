#include "./attacks/wifi/eviltwin.h"
#include "./attacks/wifi/deauth.h"
#include "./globals.h"
#include "./attacks/bluetooth/ble_scanner/ble_scanner.h"
#include "./attacks/bluetooth/gatt_server/gatt_server.h"
#include "./attacks/bluetooth/ble_spoof/ble_spoof.h"
#include "./attacks/rfid/rfid.h"
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
void handleRFIDScan(WebServer &server);
void handleRFIDWrite(WebServer &server);
void handleRFIDMagicWrite(WebServer &server);
void handleRFIDCopy(WebServer &server);


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
void stopattacks()
{
    Serial2.println("STOP_DEAUTH");
}

void handlestorage()
{
    startStorageServer(server);
}
void blescan()
{
    
    scanBLEDevices();
    String json = "[";
    for (size_t i = 0; i < bleNameList.size(); ++i)
    {
        if (i > 0)
            json += ",";
        json += "{\"name\":\"" + bleNameList[i] + "\",\"mac\":\"" + bleMacList[i] + "\"}";
    }
    json += "]";
    server.send(200, "application/json", json);
}
void gattserver()
{
    startFakeGATTServer();
    server.send(200, "application/json", "{\"status\": \"gatt_server_started\"}");
}
void handleblespoof()
{
    startBLESpoof();
    server.send(200, "application/json", "{\"status\": \"ble_spoof_started\"}");
}
void handlerfid()
{

    server.send(200, "application/json", "{\"status\": \"rfid_started\"}");
}

// ================= RFID API FUNCTIONS =================

// Scan card
void handleRFIDScan(WebServer &server)
{
    if (!RFID_CheckForCard())
    {
        server.send(404, "application/json", "{\"error\":\"No card detected\"}");
        return;
    }

    String uid = RFID_ReadUID();

    // Convert block 1 data to hex string
    String blockData = "";
    for (int i = 0; i < 16; i++)
    {
        if (lastScannedUID[i] < 0x10)
            blockData += "0";
        blockData += String(lastScannedUID[i], HEX);
    }
    blockData.toUpperCase();

    String json = "{\"uid\":\"" + uid + "\",\"block1\":\"" + blockData + "\"}";
    server.send(200, "application/json", json);
}

// Write custom block data
void handleRFIDWrite(WebServer &server)
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "application/json", "{\"error\":\"Missing JSON body\"}");
        return;
    }

    String body = server.arg("plain");
    // Expected: {"block":1,"data":"11223344556677889900AABBCCDDEEFF"}

    int blockStart = body.indexOf("\"block\":") + 8;
    int blockEnd = body.indexOf(",", blockStart);
    int block = body.substring(blockStart, blockEnd).toInt();

    int dataStart = body.indexOf("\"data\":\"") + 8;
    int dataEnd = body.indexOf("\"", dataStart);
    String dataStr = body.substring(dataStart, dataEnd);

    if (dataStr.length() != 32)
    {
        server.send(400, "application/json", "{\"error\":\"Data must be 16 bytes (32 hex chars)\"}");
        return;
    }

    byte buffer[16];
    for (int i = 0; i < 16; i++)
    {
        buffer[i] = strtoul(dataStr.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
    }

    if (RFID_WriteBlock(block, buffer))
    {
        server.send(200, "application/json", "{\"status\":\"write_success\"}");
    }
    else
    {
        server.send(500, "application/json", "{\"status\":\"write_failed\"}");
    }
}

// Write UID to magic card
void handleRFIDMagicWrite(WebServer &server)
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "application/json", "{\"error\":\"Missing JSON body\"}");
        return;
    }

    String body = server.arg("plain");
    // Expected: {"uid":"A1B2C3D4"}

    int uidStart = body.indexOf("\"uid\":\"") + 7;
    int uidEnd = body.indexOf("\"", uidStart);
    String uidStr = body.substring(uidStart, uidEnd);

    if (uidStr.length() != 8)
    {
        server.send(400, "application/json", "{\"error\":\"UID must be 4 bytes (8 hex chars)\"}");
        return;
    }

    byte uidData[16] = {0};
    for (int i = 0; i < 4; i++)
    {
        uidData[i] = strtoul(uidStr.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
    }

    if (RFID_WriteMagicUID(uidData))
    {
        server.send(200, "application/json", "{\"status\":\"uid_write_success\"}");
    }
    else
    {
        server.send(500, "application/json", "{\"status\":\"uid_write_failed\"}");
    }
}

// Copy last scanned card to magic card
void handleRFIDCopy(WebServer &server)
{
    if (RFID_CopyToMagicCard())
    {
        server.send(200, "application/json", "{\"status\":\"copy_success\"}");
    }
    else
    {
        server.send(500, "application/json", "{\"status\":\"copy_failed\"}");
    }
}

void handleRestAPI()
{
    server.on("/", handleRoot);
    server.on("/wifi/scan", handleWiFiScan);
    server.on("/wifi/eviltwin", handleEvilTwinStart);
    server.on("/wifi/deauth", handleDeauthStart);
    server.on("/wifi/stop", stopattacks);
    server.on("/storage", handlestorage);
    server.on("/bluetooth/scan", blescan);
    server.on("/bluetooth/gatt", gattserver);
    server.on("/bluetooth/spoof", handleblespoof);
    // --- RFID endpoints ---
    server.on("/rfid/scan", HTTP_GET, []()
              { handleRFIDScan(server); });
    server.on("/rfid/write", HTTP_POST, []()
              { handleRFIDWrite(server); });
    server.on("/rfid/magic/write", HTTP_POST, []()
              { handleRFIDMagicWrite(server); });
    server.on("/rfid/copy", HTTP_GET, []()
              { handleRFIDCopy(server); });
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
