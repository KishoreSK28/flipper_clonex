#ifndef UI_H
#define UI_H
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;
// Initializes OLED and button inputs
void initUI();
// Updates the UI based on button input
void updateUI();
// Renders the current menu on OLED
void displayMenu();
// Handles DNS redirection for Evil Twin captive portal
void handleDNS();
#endif // UI.H