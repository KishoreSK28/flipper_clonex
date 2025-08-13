#include "./ui.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "control_mode.h"

// External display object
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// Define physical buttons
#define BUTTON_UP 15     // black-yellow
#define BUTTON_DOWN 23   // black-green
#define BUTTON_SELECT 18 // white-blue

ControlMode currentControlMode = PHYSICAL;
int selectionIndex = 0;  // 0 for PHYSICAL, 1 for APP

void initControlButtons() {
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);
}

void displayControlModeSelection() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 12, "Select Control Mode:");

    u8g2.setDrawColor(selectionIndex == 0 ? 1 : 0);
    u8g2.drawBox(5, 25, 118, 16);
    u8g2.setDrawColor(selectionIndex == 0 ? 0 : 1);
    u8g2.drawStr(15, 36, "PHYSICAL BUTTON");

    u8g2.setDrawColor(selectionIndex == 1 ? 1 : 0);
    u8g2.drawBox(5, 45, 118, 16);
    u8g2.setDrawColor(selectionIndex == 1 ? 0 : 1);
    u8g2.drawStr(15, 56, "APP CONTROL");

    u8g2.setDrawColor(1); // Reset color
    u8g2.sendBuffer();
}

void waitForControlModeSelection() {
    initControlButtons();
    displayControlModeSelection();

    while (true) {
        if (digitalRead(BUTTON_UP) == LOW) {
            selectionIndex = max(0, selectionIndex - 1);
            displayControlModeSelection();
            delay(300);
        }

        if (digitalRead(BUTTON_DOWN) == LOW) {
            selectionIndex = min(1, selectionIndex + 1);
            displayControlModeSelection();
            delay(300);
        }

        if (digitalRead(BUTTON_SELECT) == LOW) {
            currentControlMode = (selectionIndex == 0) ? PHYSICAL : APP;
            delay(300);
            break;
        }
    }
}