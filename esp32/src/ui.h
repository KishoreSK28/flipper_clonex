#ifndef UI_H
#define UI_H
// Initializes OLED and button inputs
void initUI();
// Updates the UI based on button input
void updateUI();
// Renders the current menu on OLED
void displayMenu();
// Handles DNS redirection for Evil Twin captive portal
void handleDNS();
#endif // UI.H