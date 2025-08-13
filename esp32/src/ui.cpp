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
    RFID_SCAN,
    STORAGE_SERVER,
    APP_CONTROL,
    GATT_SERVER,
    BLE_SPOOF,
    BLE_SCANNER,
    RFID_MENU,
    RFID_READ,
    RFID_WRITE,
    RFID_SCAN
};

MenuState currentMenu = MAIN_MENU;
int menuIndex = 0;
int scrollOffset = 0;

String mainMenu[] = {"Wi-Fi Attacks", "Bluetooth Attack", "IR Attacks", "RFID attacks", "Storage Server", "APP Control"};
String wifiMenu[] = {"Evil Twin", "Deauth", "Beacon"};
String bluetoothMenu[] = {"BLE Scanner", "GATT Server", "BLE Spoof"};
String rfidMenu[] = {"RFID Scan", "RFID Read", "RFID Write"};

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
        currentMenu = RFID_MENU;
        break;
    default:
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
        else
        {
            int maxItems = (currentMenu == MAIN_MENU)        ? 6
                           : (currentMenu == WIFI_MENU)      ? 3
                           : (currentMenu == BLUETOOTH_MENU) ? 3
                           : (currentMenu == RFID_MENU)      ? 3 // ✅ Added RFID menu here
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

        else
        {
            int maxItems = (currentMenu == MAIN_MENU)        ? 6
                           : (currentMenu == WIFI_MENU)      ? 3
                           : (currentMenu == BLUETOOTH_MENU) ? 4
                           : (currentMenu == RFID_MENU)      ? 3 // ✅ Added here too
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
                // IR Attack
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

        changed = true;
        delay(200);
    }

    if (digitalRead(BUTTON_BACK) == LOW)
    {
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

        goBackMenu();
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
