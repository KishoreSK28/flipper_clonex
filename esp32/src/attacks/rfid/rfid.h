#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <vector>

// Pin mapping for ESP32
#define RST_PIN   26  // Reset
#define IRQ_PIN   27  // Optional interrupt pin
#define SS_PIN    5   // SDA / SS
#define SCK_PIN   14  // SCK
#define MISO_PIN  4   // MISO
#define MOSI_PIN  2   // MOSI
// Global RFID object
extern MFRC522 mfrc522;
extern std::vector<String> rfidTagList;
// Function prototypes
void RFID_Init();
bool RFID_CheckForCard();
String RFID_ReadUID();
bool RFID_ReadBlock(byte blockAddr, byte *buffer);
bool RFID_WriteBlock(byte blockAddr, byte *data);
bool RFID_WriteBlock0(byte *uidData); // For Chinese magic cards
void RFID_AddTag(const String &uid);

#endif