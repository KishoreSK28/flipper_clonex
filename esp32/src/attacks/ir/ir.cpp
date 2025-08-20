#include "ir.h"
#include <IRremote.hpp>

std::vector<IRSignal> irSignalList;

// ----- INIT -----
void IR_InitReceiver() {
   IrReceiver.begin(IR_RECV_PIN, ENABLE_LED_FEEDBACK);
    Serial.println("[IR] Receiver initialized");
}

void IR_InitTransmitter() {
    IrSender.begin();
    Serial.println("[IR] Transmitter initialized");
}

// ----- RECEIVE -----
bool IR_Receive(uint32_t &receivedData, int &bits) {
    if (IrReceiver.decode()) {
        receivedData = IrReceiver.decodedIRData.decodedRawData;
        bits = IrReceiver.decodedIRData.numberOfBits;

        Serial.print("[IR] Received: 0x");
        Serial.print(receivedData, HEX);
        Serial.print("  Bits: ");
        Serial.println(bits);

        IrReceiver.resume();
        return true;
    }
    return false;
}

// ----- SEND -----
void IR_Send(uint32_t data, int bits) {
    IrSender.sendNEC(data, bits);  // NEC protocol as example
    Serial.print("[IR] Sent: 0x");
    Serial.print(data, HEX);
    Serial.print("  Bits: ");
    Serial.println(bits);
}

// ----- SAVE TO SD -----
void IR_SaveSignal(uint32_t data, int bits) {
    IRSignal sig = {data, bits};
    irSignalList.push_back(sig);

    File file = SD.open("/ir_signals.txt", FILE_APPEND);
    if (file) {
        file.print(data, HEX);
        file.print(",");
        file.println(bits);
        file.close();
        Serial.println("[IR] Signal saved to SD");
    } else {
        Serial.println("[IR] Failed to open file for saving");
    }
}

// ----- LOAD FROM SD -----
void IR_LoadSignalsFromSD() {
    irSignalList.clear();

    File file = SD.open("/ir_signals.txt", FILE_READ);
    if (file) {
        while (file.available()) {
            String line = file.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
                int commaIndex = line.indexOf(',');
                if (commaIndex > 0) {
                    String dataStr = line.substring(0, commaIndex);
                    String bitsStr = line.substring(commaIndex + 1);

                    uint32_t data = (uint32_t) strtoul(dataStr.c_str(), NULL, 16);
                    int bits = bitsStr.toInt();

                    irSignalList.push_back({data, bits});
                }
            }
        }
        file.close();
        Serial.print("[IR] Loaded signals: ");
        Serial.println(irSignalList.size());
    } else {
        Serial.println("[IR] No signal file found, starting fresh.");
    }
}

// ----- REPLAY -----
void IR_ReplaySignal(int index) {
    if (index >= 0 && index < irSignalList.size()) {
        IRSignal sig = irSignalList[index];
        IR_Send(sig.data, sig.bits);
        Serial.println("[IR] Replayed from SD list");
    } else {
        Serial.println("[IR] Invalid signal index");
    }
}
