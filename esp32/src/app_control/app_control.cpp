#include <WiFi.h>
#include <ArduinoJson.h> // Using modern ArduinoJson v7+

// Include headers for all feature modules
#include "attacks/wifi/eviltwin.h"
#include "attacks/wifi/deauth.h"
#include "attacks/wifi/wifiscan.h"
#include "attacks/bluetooth/ble_scanner/ble_scanner.h"
#include "attacks/bluetooth/gatt_server/gatt_server.h"
#include "attacks/bluetooth/ble_spoof/ble_spoof.h"
#include "attacks/rfid/rfid.h"
#include "storage/server.h"
#include "globals.h"



// --- Function Prototypes ---
void handleRoot();
void handleWiFiScan();
void handleEvilTwinStart();
void handleDeauthStart();
void stopAttacks();
void blescan();
void gattserver();
void handleblespoof();
void handleRFIDScan();
void handleRFIDWrite();
void handleRFIDMagicWrite();
void handleRFIDCopy();
void handleRestAPI();

// --- Server & Hardware Initialization ---
void initAppControlMode() {
    WiFi.softAP("Flipper Clonex", "123123123");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP ADDRESS: ");
    Serial.println(IP);

    // Initialize RFID reader
    RFID_Init();

    // Set up and start the web server and API endpoints
    handleRestAPI();
    server.begin();
}

void handleRoot() {
    server.send(200, "text/plain", "FlipperCloneX API Online");
}

// --- Wi-Fi Functions ---
void handleWiFiScan() {
    // Call the function from wifiscan.cpp
    scanWiFiNetworks();
    
    JsonDocument doc;
    JsonArray networks = doc.to<JsonArray>();

    for (int i = 0; i < apCount; ++i) {
        JsonObject network = networks.add<JsonObject>();
        network["ssid"] = ssidList[i];
        
        char bssidStr[18];
        sprintf(bssidStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                bssidList[i][0], bssidList[i][1], bssidList[i][2],
                bssidList[i][3], bssidList[i][4], bssidList[i][5]);
        network["bssid"] = bssidStr;
        
        // Note: RSSI is not stored in your wifiscan.cpp, so it's omitted here.
        network["channel"] = channelList[i];
    }

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void handleEvilTwinStart() {
    // Call the function from eviltwin.cpp
    startEvilTwin();
    server.send(200, "application/json", "{\"status\":\"evil_twin_started\"}");
}

void handleDeauthStart() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing JSON\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    const char* bssidStr = doc["bssid"];
    int channel = doc["channel"];

    if (!bssidStr || channel == 0) {
        server.send(400, "application/json", "{\"error\":\"Missing bssid or channel\"}");
        return;
    }

    uint8_t bssid[6];
    sscanf(bssidStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);
    
    // Call the function from deauth.cpp
    startDeauth(bssid, channel);

    server.send(200, "application/json", "{\"status\": \"deauth_started\"}");
}

void stopAttacks() {
    // Call the function from deauth.cpp
    stopDeauth();
    // You might add other stop functions here in the future
    server.send(200, "application/json", "{\"status\":\"attacks_stopped\"}");
}

// --- Bluetooth Functions ---
void blescan() {
    // Call the function from ble_scanner.cpp
    scanBLEDevices();
    
    JsonDocument doc;
    JsonArray devices = doc.to<JsonArray>();
    
    for (size_t i = 0; i < bleNameList.size(); ++i) {
        JsonObject device = devices.add<JsonObject>();
        device["name"] = bleNameList[i];
        device["mac"] = bleMacList[i];
    }
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void gattserver() {
    // Call the function from gatt_server.cpp
    startFakeGATTServer();
    server.send(200, "application/json", "{\"status\":\"gatt_server_started\"}");
}

void handleblespoof() {
    // Call the function from ble_spoof.cpp
    startBLESpoof();
    server.send(200, "application/json", "{\"status\":\"ble_spoof_started\"}");
}

// --- RFID Functions ---
void handleRFIDScan() {
    // Calls functions from rfid.cpp
    if (!RFID_CheckForCard()) {
        server.send(404, "application/json", "{\"error\":\"No card detected\"}");
        return;
    }

    String uid = RFID_ReadUID();
    String blockData = "";
    for (int i = 0; i < 16; i++) {
        if (lastScannedUID[i] < 0x10) blockData += "0";
        blockData += String(lastScannedUID[i], HEX);
    }
    blockData.toUpperCase();
    
    JsonDocument doc;
    doc["uid"] = uid;
    doc["block1"] = blockData;

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void handleRFIDWrite() {
    if (!server.hasArg("plain")) { /* ... */ return; }

    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));

    int block = doc["block"];
    const char* dataStr = doc["data"];
    // ... (rest of parsing logic) ...
    byte buffer[16];
    // ...
    
    // Calls function from rfid.cpp
    if (RFID_WriteBlock(block, buffer)) {
        server.send(200, "application/json", "{\"status\":\"write_success\"}");
    } else {
        server.send(500, "application/json", "{\"status\":\"write_failed\"}");
    }
}

void handleRFIDMagicWrite() {
    if (!server.hasArg("plain")) { /* ... */ return; }

    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    const char* uidStr = doc["uid"];
    // ... (rest of parsing logic) ...
    byte uidData[4] = {0};
    // ...

    // Calls function from rfid.cpp
    if (RFID_WriteMagicUID(uidData)) {
        server.send(200, "application/json", "{\"status\":\"uid_write_success\"}");
    } else {
        server.send(500, "application/json", "{\"status\":\"uid_write_failed\"}");
    }
}

void handleRFIDCopy() {
    // Calls function from rfid.cpp
    if (RFID_CopyToMagicCard()) {
        server.send(200, "application/json", "{\"status\":\"copy_success\"}");
    } else {
        server.send(500, "application/json", "{\"status\":\"copy_failed\"}");
    }
}

// --- API Route Registration ---
void handleRestAPI() {
    server.on("/", HTTP_GET, handleRoot);

    // Wi-Fi Routes
    server.on("/wifi/scan", HTTP_GET, handleWiFiScan);
    server.on("/wifi/eviltwin", HTTP_GET, handleEvilTwinStart);
    server.on("/wifi/eviltwin/credentials", HTTP_GET, handleGetCredentials);
    server.on("/wifi/deauth", HTTP_POST, handleDeauthStart);
    server.on("/wifi/stop", HTTP_GET, stopAttacks);
    
    // Bluetooth Routes
    server.on("/bluetooth/scan", HTTP_GET, blescan);
    server.on("/bluetooth/gatt", HTTP_GET, gattserver);
    server.on("/bluetooth/spoof", HTTP_GET, handleblespoof);

    // RFID Routes
    server.on("/rfid/scan", HTTP_GET, handleRFIDScan);
    server.on("/rfid/write", HTTP_POST, handleRFIDWrite);

    server.on("/rfid/magic/write", HTTP_POST, handleRFIDMagicWrite);
    server.on("/rfid/copy", HTTP_GET, handleRFIDCopy);
}