#include "rfid.h"

MFRC522 mfrc522(SS_PIN, RST_PIN);

void RFID_Init()
{
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    mfrc522.PCD_Init();
    Serial.println("[RFID] Reader initialized");
}

bool RFID_CheckForCard()
{
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
        String uid = RFID_ReadUID();
        RFID_AddTag(uid);
        return true;
    }
    return false;
}

String RFID_ReadUID()
{
    String uidStr;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        if (mfrc522.uid.uidByte[i] < 0x10)
            uidStr += "0";
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    return uidStr;
}

bool RFID_ReadBlock(byte blockAddr, byte *buffer)
{
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;
    byte size = 18;

    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK)
    {
        Serial.println("[RFID] Auth failed");
        return false;
    }
    if (mfrc522.MIFARE_Read(blockAddr, buffer, &size) != MFRC522::STATUS_OK)
    {
        Serial.println("[RFID] Read failed");
        return false;
    }
    return true;
}

bool RFID_WriteBlock(byte blockAddr, byte *data)
{
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK)
    {
        Serial.println("[RFID] Auth failed");
        return false;
    }
    if (mfrc522.MIFARE_Write(blockAddr, data, 16) != MFRC522::STATUS_OK)
    {
        Serial.println("[RFID] Write failed");
        return false;
    }
    return true;
}

// For Chinese magic cards (UID writable)
bool RFID_WriteBlock0(byte *uidData)
{
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Idle);
    delay(10);
    if (mfrc522.MIFARE_Ultralight_Write(0, uidData, 16) != MFRC522::STATUS_OK)
    {
        Serial.println("[RFID] Block 0 write failed (UID change)");
        return false;
    }
    Serial.println("[RFID] UID successfully changed");
    return true;
}
std::vector<String> rfidTagList;

void RFID_AddTag(const String &uid)
{
    // Avoid duplicates
    if (std::find(rfidTagList.begin(), rfidTagList.end(), uid) == rfidTagList.end())
    {
        rfidTagList.push_back(uid);
    }
}
