#include "rfid.h"

String lastScannedUID = "";  // Store the last scanned UID
MFRC522 mfrc522(SS_PIN, RST_PIN);
std::vector<String> rfidTagList;

// Store the last scanned normal card UID

byte lastScannedData[16];  // store one block data (for example block 1)

// --- Initialize RFID ---
void RFID_Init() {
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    mfrc522.PCD_Init();
    Serial.println("[RFID] Reader initialized");
}

// --- Check for any card ---
bool RFID_CheckForCard() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String uid = RFID_ReadUID();
        RFID_AddTag(uid);
        lastScannedUID = uid;

        // Example: Read block 1 data (can be adjusted)
        if (!RFID_ReadBlock(1, lastScannedData)) {
            Serial.println("[RFID] Failed to read data block from card");
        }

        return true;
    }
    return false;
}

// --- Read UID ---
String RFID_ReadUID() {
    String uidStr;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    return uidStr;
}

// --- Read block data ---
bool RFID_ReadBlock(byte blockAddr, byte *buffer) {
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
    byte size = 18;

    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
        Serial.println("[RFID] Auth failed");
        return false;
    }
    if (mfrc522.MIFARE_Read(blockAddr, buffer, &size) != MFRC522::STATUS_OK) {
        Serial.println("[RFID] Read failed");
        return false;
    }
    return true;
}

// --- Write block (normal) ---
bool RFID_WriteBlock(byte blockAddr, byte *data) {
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
        Serial.println("[RFID] Auth failed");
        return false;
    }
    if (mfrc522.MIFARE_Write(blockAddr, data, 16) != MFRC522::STATUS_OK) {
        Serial.println("[RFID] Write failed");
        return false;
    }
    return true;
}

// --- Write UID to magic card ---
bool RFID_WriteMagicUID(byte *uidData) {
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Idle);
    delay(10);
    if (mfrc522.MIFARE_Ultralight_Write(0, uidData, 16) != MFRC522::STATUS_OK) {
        Serial.println("[RFID] Block 0 write failed (UID change)");
        return false;
    }
    Serial.println("[RFID] UID successfully changed");
    return true;
}

// --- Copy last scanned card to magic card ---
bool RFID_CopyToMagicCard() {
    String uid = RFID_ReadUID();

    // Detect if current card is magic (basic check)
    if (mfrc522.uid.size != 4) {  // normal magic cards usually 4-byte UID
        Serial.println("[RFID] Not a magic card, copy aborted");
        return false;
    }

    // Prepare UID data to write (example: use lastScannedUID)
    byte uidData[16] = {0};
    for (byte i = 0; i < 4; i++) { // only first 4 bytes for UID
        uidData[i] = strtoul(lastScannedUID.substring(i*2, i*2+2).c_str(), NULL, 16);
    }

    // Write UID
    if (!RFID_WriteMagicUID(uidData)) return false;

    // Optionally copy block 1 data
    if (!RFID_WriteBlock(1, lastScannedData)) {
        Serial.println("[RFID] Warning: could not copy block data");
    }

    Serial.println("[RFID] Copy to magic card completed!");
    return true;
}

// --- Store UID in list ---
void RFID_AddTag(const String &uid) {
    if (std::find(rfidTagList.begin(), rfidTagList.end(), uid) == rfidTagList.end()) {
        rfidTagList.push_back(uid);
    }
}
