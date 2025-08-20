#include "ui.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include "storage/server.h"
#include "globals.h"
#include "attacks/wifi/eviltwin.h"
#include "attacks/wifi/wifiscan.h"
#include "attacks/bluetooth/ble_scanner/ble_scanner.h"
#include "attacks/bluetooth/gatt_server/gatt_server.h"
#include "attacks/bluetooth/ble_spoof/ble_spoof.h"
#include "app_control/app_control.h"
#include "attacks/rfid/rfid.h"
#include "attacks/ir/ir.h"

extern String ssid;
extern String pass;
extern bool capnew;
extern String ssidList[20];
extern uint8_t bssidList[20][6];
extern int channelList[20];
extern int apCount;
String formatBSSID(uint8_t *bssid);
extern HardwareSerial Serial2; // for esp8266 command

// OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

// Button Pins
#define BUTTON_UP 15     // black-yellow
#define BUTTON_DOWN 23   // black-green
#define BUTTON_SELECT 18 // white-blue
#define BUTTON_BACK 19   // red-grey

enum MenuState
{
    MAIN_MENU,
    WIFI_MENU,
    BLUETOOTH_MENU,
    EVIL_TWIN_MENU,
    DEAUTH_MENU,
    BEACON_MENU,
    SELECT_WIFI_FOR_DEAUTH,
    STORAGE_SERVER,
    APP_CONTROL,
    GATT_SERVER,
    BLE_SPOOF,
    BLE_SCANNER,
    RFID_MENU,
    RFID_READ,
    RFID_WRITE,
    RFID_SCAN,
    RFID_WRITE_MAGIC,
    RFID_COPY_TO_MAGIC_CARD,
    IR_MENU,
    IR_RECEIVE,
    IR_REPLAY,
    IR_SEND
};

MenuState currentMenu = MAIN_MENU;
int menuIndex = 0;
int scrollOffset = 0;

String mainMenu[] = {"Wi-Fi Attacks", "Bluetooth Attack", "IR Attacks", "RFID attacks", "Storage Server", "APP Control"};
String wifiMenu[] = {"Evil Twin", "Deauth", "Beacon"};
String bluetoothMenu[] = {"BLE Scanner", "GATT Server", "BLE Spoof"};
String irmenu[] = {"Receive IR", "Replay IR", "Send IR"};
String rfidMenu[] = {"RFID Scan", "RFID Read", "RFID Write", "RFID Write Magic card", "Copy to Magic Card"};

void initUI()
{
    u8g2.begin();
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);
    pinMode(BUTTON_BACK, INPUT_PULLUP);
    displayMenu();
    esp_log_level_set("*", ESP_LOG_WARN);     // Lower global logging
    esp_log_level_set("wifi", ESP_LOG_ERROR); // Only show real errors from Wi-Fi
}

void goBackMenu()
{
    switch (currentMenu)
    {
    case WIFI_MENU:
        currentMenu = MAIN_MENU;
        break;
    case EVIL_TWIN_MENU:
    case DEAUTH_MENU:
    case BEACON_MENU:
    case SELECT_WIFI_FOR_DEAUTH:
        currentMenu = WIFI_MENU;
        break;
    case BLUETOOTH_MENU:
        currentMenu = MAIN_MENU;
        break;
    case GATT_SERVER:
    case BLE_SPOOF:
    case BLE_SCANNER:
        currentMenu = BLUETOOTH_MENU;
        break;
    case RFID_SCAN:
    case RFID_READ:
    case RFID_WRITE:
    case RFID_WRITE_MAGIC:
    case RFID_COPY_TO_MAGIC_CARD:
        currentMenu = RFID_MENU;
        break;
    case IR_RECEIVE:
    case IR_REPLAY:
    case IR_SEND:
        currentMenu = IR_MENU;
        break;
    case STORAGE_SERVER:
        currentMenu = MAIN_MENU;
        break;

    case APP_CONTROL:
        currentMenu = MAIN_MENU;
        break;

    default:
        currentMenu = MAIN_MENU;
        break;
    }
    menuIndex = 0;
    scrollOffset = 0;
}

void updateUI()
{
    bool changed = false;

    if (digitalRead(BUTTON_UP) == LOW)
    {
        if (currentMenu == SELECT_WIFI_FOR_DEAUTH)
        {
            if (menuIndex > 0)
                menuIndex--;
            if (menuIndex < scrollOffset)
                scrollOffset--;
        }
        else if (currentMenu == BLE_SCANNER)
        {
            if (menuIndex > 0)
                menuIndex--;
            if (menuIndex < scrollOffset)
                scrollOffset--;
        }
        else if (currentMenu == RFID_SCAN)
        {
            if (menuIndex > 0)
                menuIndex--;
            if (menuIndex < scrollOffset)
                scrollOffset--;
        }
        else if (currentMenu == IR_MENU)
        {
            if (menuIndex > 0)
                menuIndex--;
            if (menuIndex < scrollOffset)
                scrollOffset--;
        }
        else
        {
            int maxItems = (currentMenu == MAIN_MENU)        ? 6
                           : (currentMenu == WIFI_MENU)      ? 3
                           : (currentMenu == BLUETOOTH_MENU) ? 3
                           : (currentMenu == RFID_MENU)      ? 5 // ✅ Added RFID menu here
                           : (currentMenu == IR_MENU)        ? 3
                                                             : 6;

            if (menuIndex > 0)
                menuIndex--;
            if ((currentMenu == MAIN_MENU || currentMenu == BLUETOOTH_MENU || currentMenu == RFID_MENU) && menuIndex < scrollOffset)
                scrollOffset--;
        }
        changed = true;
        delay(200);
    }

    if (digitalRead(BUTTON_DOWN) == LOW)
    {
        if (currentMenu == SELECT_WIFI_FOR_DEAUTH)
        {
            if (menuIndex < apCount - 1)
                menuIndex++;
            if (menuIndex >= scrollOffset + 4)
                scrollOffset++;
        }
        else if (currentMenu == BLE_SCANNER)
        {
            if (menuIndex < (int)bleNameList.size() - 1)
                menuIndex++;
            if (menuIndex >= scrollOffset + 4)
                scrollOffset++;
        }
        else if (currentMenu == RFID_MENU) // ✅ Added RFID menu scrolling
        {
            if (menuIndex < (int)rfidTagList.size() - 1) // rfidTagList = vector of scanned tags
                menuIndex++;
            if (menuIndex >= scrollOffset + 4)
                scrollOffset++;
        }
        else if (currentMenu == IR_MENU)
        {
            if (menuIndex < (sizeof(irmenu) / sizeof(irmenu[0])) - 1) // limit to menu size
                menuIndex++;

            if (menuIndex >= scrollOffset + 4) // keep scrolling window
                scrollOffset++;
        }

        else
        {
            int maxItems = (currentMenu == MAIN_MENU)        ? 6
                           : (currentMenu == WIFI_MENU)      ? 3 // ✅ Added Wi-Fi menu item count
                           : (currentMenu == BLUETOOTH_MENU) ? 4 // ✅ Added BLE menu item count
                           : (currentMenu == RFID_MENU)      ? 5 // ✅ Added here too
                           : (currentMenu == IR_MENU)        ? 3 // ✅ Added IR menu
                                                             : 6;

            if (menuIndex < maxItems - 1)
                menuIndex++;

            if ((currentMenu == MAIN_MENU || currentMenu == BLUETOOTH_MENU) && menuIndex >= scrollOffset + 4)
                scrollOffset++;
        }
        changed = true;
        delay(200);
    }

    if (digitalRead(BUTTON_SELECT) == LOW)
    {
        if (currentMenu == MAIN_MENU)
        {
            if (menuIndex == 0)
            {
                currentMenu = WIFI_MENU;
            }
            else if (menuIndex == 1)
            {
                currentMenu = BLUETOOTH_MENU;
            }
            else if (menuIndex == 2)
            {
                currentMenu = IR_MENU;
            }
            else if (menuIndex == 3)
            {
                currentMenu = RFID_SCAN;
            }
            else if (menuIndex == 4)
            {
                WiFi.softAP("ESP32_AP", "12345678");
                startStorageServer(server);
                Serial.println("[STORAGE] Server started at 192.168.4.1");
                currentMenu = STORAGE_SERVER;
            }
            else if (menuIndex == 5)
            {
                initAppControlMode();
                currentMenu = APP_CONTROL;
            }
        }
        else if (currentMenu == WIFI_MENU)
        {
            if (menuIndex == 0)
            {
                currentMenu = EVIL_TWIN_MENU;
                startEvilTwin();
            }
            else if (menuIndex == 1)
            {
                u8g2.clearBuffer();
                u8g2.setFont(u8g2_font_6x10_tf);
                u8g2.setCursor(10, 30);
                u8g2.print("Scanning Wi-Fi...");
                u8g2.sendBuffer();

                delay(500);

                scanWiFiNetworks();
                menuIndex = 0;
                scrollOffset = 0;
                currentMenu = SELECT_WIFI_FOR_DEAUTH;
            }
            else if (menuIndex == 2)
            {
                Serial2.println("START_BEACON_FLOOD");
                Serial.println("[esp32] Sent: Start_BEACON_FLOOD");
                currentMenu = BEACON_MENU;
            }
        }
        else if (currentMenu == BLUETOOTH_MENU)
        {
            if (menuIndex == 0)
            {
                u8g2.clearBuffer();
                u8g2.setFont(u8g2_font_6x10_tf);
                u8g2.setCursor(10, 30);
                u8g2.print("Scanning BLE...");
                u8g2.sendBuffer();

                delay(500);

                scanBLEDevices();
                menuIndex = 0;
                scrollOffset = 0;
                scanBLEDevices();
                currentMenu = BLE_SCANNER;
            }
            else if (menuIndex == 1)
            {

                startFakeGATTServer();
                currentMenu = GATT_SERVER;
            }
            else if (menuIndex == 2)
            {
                startBLESpoof();
                currentMenu = BLE_SPOOF;
            }
        }

        else if (currentMenu == SELECT_WIFI_FOR_DEAUTH)
        {
            String selectedSSID = ssidList[menuIndex];
            uint8_t *bssid = bssidList[menuIndex];
            int channel = channelList[menuIndex];

            Serial.println("[DEAUTH] Target SSID: " + selectedSSID);
            Serial.println("[DEAUTH] Target BSSID: " + formatBSSID(bssid));

            String bssidStr = formatBSSID(bssid);
            Serial2.println("START_DEAUTH_BROADCAST " + bssidStr + " " + String(channel));
            Serial.println("[ESP32] Sent: START_DEAUTH " + bssidStr + " " + String(channel));
            currentMenu = DEAUTH_MENU;
        }

        else if (currentMenu == IR_MENU)
        {
            if (menuIndex == 0)
            {
                // ---- IR Receive & Store ----
                IR_InitReceiver();
                uint32_t receivedData;
                int bits;

                if (IR_Receive(receivedData, bits))
                {
                    IR_SaveSignal(receivedData, bits);
                    Serial.println("IR signal received and stored to SD!");
                }
                else
                {
                    Serial.println("No IR signal received!");
                }
            }
            else if (menuIndex == 1)
            {
                // ---- IR Replay ----
                IR_InitTransmitter();
                if (!irSignalList.empty())
                {
                    IR_ReplaySignal(irSignalList.size() - 1); // replay last stored
                    Serial.println("IR signal replayed!");
                }
                else
                {
                    Serial.println("No stored signals to replay!");
                }
            }
            else if (menuIndex == 2)
            {
                // ---- IR Send (manual example) ----
                IR_InitTransmitter();
                uint32_t testData = 0x1FE48B7; // Example IR code
                int bits = 32;

                IR_Send(testData, bits);
                Serial.println("Custom IR signal sent!");
            }
        }
        else if (currentMenu == RFID_MENU)
        {
            RFID_Init(); // Initialize RFID hardware once

            if (menuIndex == 0) // Scan normal card
            {
                RFID_Init();
                currentMenu = RFID_SCAN;
                if (RFID_CheckForCard())
                {
                    String uid = RFID_ReadUID();
                    RFID_AddTag(uid);     // Add UID to the tag list
                    lastScannedUID = uid; // store last scanned UID
                    Serial.println("[RFID] Card detected, UID: " + uid);
                }
                else
                {
                    Serial.println("[RFID] No card detected.");
                }
            }
            else if (menuIndex == 1) // Read first safe block (example: block 1)
            {
                currentMenu = RFID_READ;
                if (RFID_CheckForCard())
                {
                    byte buffer[16];
                    byte blockAddr = 1; // safe block example
                    if (RFID_ReadBlock(blockAddr, buffer))
                    {
                        Serial.print("[RFID] Block ");
                        Serial.print(blockAddr);
                        Serial.print(": ");
                        for (int i = 0; i < 16; i++)
                            Serial.printf("%02X ", buffer[i]);
                        Serial.println();
                    }
                    else
                    {
                        Serial.println("[RFID] Failed to read block.");
                    }
                }
                else
                {
                    Serial.println("[RFID] No card detected.");
                }
            }
            else if (menuIndex == 2) // Write example data to first safe block
            {
                currentMenu = RFID_WRITE;
                if (RFID_CheckForCard())
                {
                    byte dataToWrite[16] = {0xDE, 0xAD, 0xBE, 0xEF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                    byte blockAddr = 1; // first safe block
                    if (RFID_WriteBlock(blockAddr, dataToWrite))
                    {
                        Serial.println("[RFID] Write successful.");
                    }
                    else
                    {
                        Serial.println("[RFID] Write failed.");
                    }
                }
                else
                {
                    Serial.println("[RFID] No card detected.");
                }
            }
            else if (menuIndex == 3) // Copy UID to magic card
            {
                currentMenu = RFID_WRITE_MAGIC;
                if (RFID_CheckForCard())
                {
                    if (RFID_CopyToMagicCard()) // uses lastScannedUID
                    {
                        Serial.println("[RFID] Magic card updated successfully.");
                    }
                    else
                    {
                        Serial.println("[RFID] Failed to update magic card.");
                    }
                }
                else
                {
                    Serial.println("[RFID] No card detected.");
                }
            }
            else if (menuIndex == 4) // Full card clone using UID only
            {
                currentMenu = RFID_COPY_TO_MAGIC_CARD;
                if (RFID_CopyToMagicCard) // this will copy UID + optional block if available
                {
                    Serial.println("[RFID] Full card cloned successfully (UID only)!");
                }
                else
                {
                    Serial.println("[RFID] Clone failed.");
                }
            }
        }

        changed = true;
        delay(200);
    }

    if (digitalRead(BUTTON_BACK) == LOW)
    {
        // Wi-Fi stop actions
        if (currentMenu == DEAUTH_MENU)
        {
            Serial2.println("STOP_DEAUTH");
            Serial.println("[ESP32] Sent: STOP_DEAUTH");
        }
        else if (currentMenu == BEACON_MENU)
        {
            Serial2.println("STOP_BEACON");
            Serial.println("[ESP32] Sent: STOP_BEACON");
        }

        // Optional: RFID actions on back
        else if (currentMenu == RFID_SCAN || currentMenu == RFID_READ || currentMenu == RFID_WRITE ||
                 currentMenu == RFID_WRITE_MAGIC || currentMenu == RFID_COPY_TO_MAGIC_CARD)
        {
            Serial.println("[RFID] Returning to RFID menu...");
            // You can add extra cleanup if needed
        }

        // Optional: IR actions on back
        else if (currentMenu == IR_RECEIVE || currentMenu == IR_REPLAY || currentMenu == IR_SEND)
        {
            Serial.println("[IR] Returning to IR menu...");
            // You can stop IR receiver or transmitter if needed
            // e.g., IR_StopReceiver() or IR_StopTransmitter() if implemented
        }

        goBackMenu(); // Handles all menu transitions
        changed = true;
        delay(200);
    }

    static bool lastCapNew = false;
    if (capnew && !lastCapNew && currentMenu == EVIL_TWIN_MENU)
    {
        displayMenu();
    }
    lastCapNew = capnew;

    if (changed)
        displayMenu();
}

void displayMenu()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(10, 10);

    if (currentMenu == MAIN_MENU)
    {
        u8g2.print("Main Menu:");
        int visibleCount = 4;
        int totalItems = 6;
        for (int i = 0; i < visibleCount; i++)
        {
            int idx = scrollOffset + i;
            if (idx >= totalItems)
                break;

            u8g2.setCursor(10, 20 + i * 10);
            if (idx == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");
            u8g2.print(mainMenu[idx]);
        }
    }
    else if (currentMenu == WIFI_MENU)
    {
        u8g2.print("Wi-Fi Attacks:");
        for (int i = 0; i < 3; i++)
        {
            u8g2.setCursor(10, 20 + i * 10);
            if (i == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");
            u8g2.print(wifiMenu[i]);
        }
    }
    else if (currentMenu == BLUETOOTH_MENU)
    {
        u8g2.print("Bluetooth Attacks:");
        for (int i = 0; i < 4; i++)
        {
            u8g2.setCursor(10, 20 + i * 10);

            if (i == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");
            u8g2.print(bluetoothMenu[i]);
        }
    }
    else if (currentMenu == IR_MENU)
    {
        u8g2.print("IR Attacks:");
        for (int i = 0; i < 3; i++)
        {
            u8g2.setCursor(10, 20 + i * 10);

            if (i == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");
            u8g2.print(irmenu[i]);
        }
    }
    else if (currentMenu == RFID_MENU)
    {
        u8g2.print("RFID Attacks:");
        for (int i = 0; i < 5; i++)
        {
            u8g2.setCursor(10, 20 + i * 10);

            if (i == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");
            u8g2.print(rfidMenu[i]);
        }
    }
    else if (currentMenu == STORAGE_SERVER)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Storage Server");
        u8g2.setCursor(0, 20);
        u8g2.print("SSID: ESP32_AP");
        u8g2.setCursor(0, 30);
        u8g2.print("Pass: 12345678");
        u8g2.setCursor(0, 40);
        u8g2.print("IP: ");
        u8g2.print(WiFi.softAPIP());
    }
    else if (currentMenu == APP_CONTROL)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("APP Control");
        u8g2.setCursor(0, 20);
        u8g2.print("SSID: Flipper Clonex");
        u8g2.setCursor(0, 30);
        u8g2.print("Pass: 123123123");
        u8g2.setCursor(0, 40);
        u8g2.print("IP: ");
        u8g2.print(WiFi.softAPIP());
    }
    else if (currentMenu == BLE_SCANNER)
    {
        u8g2.print("BLE Devices:");
        for (int i = 0; i < 4; ++i) // show up to 4 items on screen
        {
            int idx = scrollOffset + i;
            if (idx >= bleNameList.size())
                break;

            u8g2.setCursor(0, 20 + i * 10);

            if (idx == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");

            String name = bleNameList[idx];
            String mac = bleMacList[idx];

            // Truncate name if too long
            if (name.length() > 8)
            {
                name = name.substring(0, 8) + "...";
            }

            u8g2.print(name);
            u8g2.print(" ");
            u8g2.print(mac);
        }
    }
    else if (currentMenu == GATT_SERVER)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("GATT Server Active");
        u8g2.setCursor(0, 20);
        u8g2.print("Listening for BLE");
        u8g2.setCursor(0, 30);
        u8g2.print("connections...");
    }
    else if (currentMenu == BLE_SPOOF)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("BLE Spoofing Active");
        u8g2.setCursor(0, 20);
        u8g2.print("Advertising fake");
        u8g2.setCursor(0, 30);
        u8g2.print("device...");
    }
    else if (currentMenu == BLE_SPOOF)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("BLE Spoofing Active");
        u8g2.setCursor(0, 20);
        u8g2.print("Advertising fake");
        u8g2.setCursor(0, 30);
        u8g2.print("device...");
    }
    else if (currentMenu == BEACON_MENU)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Beacon Flooding Active");
        u8g2.setCursor(0, 20);
        u8g2.print("Sending beacons on all channels");
    }

    else if (currentMenu == EVIL_TWIN_MENU)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Evil Twin Active");

        if (capnew)
        {
            u8g2.setCursor(0, 25);
            u8g2.print("Email:");
            if (ssid.length() > 11)
            {
                u8g2.setCursor(40, 25);
                u8g2.print(ssid.substring(0, 11));
                u8g2.setCursor(40, 35);
                u8g2.print(ssid.substring(11));
            }
            else
            {
                u8g2.setCursor(40, 25);
                u8g2.print(ssid);
            }

            u8g2.setCursor(0, 45);
            u8g2.print("Pass:");
            if (pass.length() > 11)
            {
                u8g2.setCursor(40, 45);
                u8g2.print(pass.substring(0, 11));
                u8g2.setCursor(40, 55);
                u8g2.print(pass.substring(11));
            }
            else
            {
                u8g2.setCursor(40, 45);
                u8g2.print(pass);
            }

            capnew = false;
        }
        else
        {
            u8g2.setCursor(0, 35);
            u8g2.print("Waiting for login...");
        }
    }
    else if (currentMenu == SELECT_WIFI_FOR_DEAUTH)
    {
        u8g2.print("Select Target:");
        for (int i = 0; i < 4; ++i)
        {
            int idx = scrollOffset + i;
            if (idx >= apCount)
                break;

            u8g2.setCursor(0, 20 + i * 10);
            if (idx == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");
            u8g2.print(ssidList[idx]);
        }
    }
    else if (currentMenu == DEAUTH_MENU)
    {
        u8g2.print("Deauth Running:");
        u8g2.setCursor(2, 30);
        u8g2.print("Packets Sent:");
    }
    else if (currentMenu == RFID_MENU)
    {
        u8g2.print("RFID Menu:");
        for (int i = 0; i < 5; ++i) // up to 5 items on screen
        {
            int idx = scrollOffset + i;
            if (idx >= sizeof(rfidMenu) / sizeof(rfidMenu[0]))
                break;

            u8g2.setCursor(0, 20 + i * 10);

            if (idx == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");

            u8g2.print(rfidMenu[idx]);
        }
    }
    else if (currentMenu == RFID_SCAN)
    {
        u8g2.print("RFID Tags:");
        for (int i = 0; i < 4; ++i) // up to 4 lines on screen
        {
            int idx = scrollOffset + i;
            if (idx >= rfidTagList.size())
                break;

            u8g2.setCursor(0, 20 + i * 10);

            if (idx == menuIndex)
                u8g2.print("> ");
            else
                u8g2.print("  ");

            String uid = rfidTagList[idx];
            if (uid.length() > 12) // shorten long UIDs
                uid = uid.substring(0, 12) + "...";

            u8g2.print(uid);
        }
    }
    else if (currentMenu == RFID_COPY_TO_MAGIC_CARD)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Copying to Magic Card...");
        u8g2.setCursor(0, 20);
        u8g2.print("UID: ");
        if (lastScannedUID.length() > 12)
        {
            u8g2.print(lastScannedUID.substring(0, 12) + "...");
        }
        else
        {
            u8g2.print(lastScannedUID);
        }
    }
    else if (currentMenu == RFID_READ)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Reading RFID Card...");
        u8g2.setCursor(0, 20);
        u8g2.print("UID: ");
        if (lastScannedUID.length() > 12)
        {
            u8g2.print(lastScannedUID.substring(0, 12) + "...");
        }
        else
        {
            u8g2.print(lastScannedUID);
        }
    }
    else if (currentMenu == RFID_WRITE)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Writing to RFID Card...");
        u8g2.setCursor(0, 20);
        u8g2.print("UID: ");
        if (lastScannedUID.length() > 12)
        {
            u8g2.print(lastScannedUID.substring(0, 12) + "...");
        }
        else
        {
            u8g2.print(lastScannedUID);
        }
    }
    else if (currentMenu == RFID_WRITE_MAGIC)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Writing to Magic Card...");
        u8g2.setCursor(0, 20);
        u8g2.print("UID: ");
        if (lastScannedUID.length() > 12)
        {
            u8g2.print(lastScannedUID.substring(0, 12) + "...");
        }
        else
        {
            u8g2.print(lastScannedUID);
        }
    }
    else if (currentMenu == STORAGE_SERVER)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("Storage Server:");
        u8g2.setCursor(0, 30);
        u8g2.print("SSID: ESP32_AP");
        u8g2.setCursor(0, 40);
        u8g2.print("Pass: 12345678");
        u8g2.setCursor(0, 50);
        u8g2.print("IP: ");
        u8g2.print(WiFi.softAPIP());
    }
    else if (currentMenu == APP_CONTROL)
    {
        u8g2.setCursor(0, 10);
        u8g2.print("APP control");
        u8g2.setCursor(0, 30);
        u8g2.print("SSID: Flipper Clonex");
        u8g2.setCursor(0, 40);
        u8g2.print("Pass: 123123123");
        u8g2.setCursor(0, 50);
        u8g2.print("IP: ");
        u8g2.print(WiFi.softAPIP());
    }

    u8g2.sendBuffer();
}

String formatBSSID(uint8_t *bssid)
{
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            bssid[0], bssid[1], bssid[2],
            bssid[3], bssid[4], bssid[5]);
    return String(macStr);
}
