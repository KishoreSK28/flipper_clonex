#ifndef IR_H
#define IR_H

#include <Arduino.h>
#include <SD.h>
#include <vector>

// ----- CONFIG -----
#define IR_RECV_PIN  13   // IR receiver pin
#define IR_SEND_PIN  35  // IR transmitter pin
#define CAPTURE_BUFFER_SIZE 1024
#define TIMEOUT 50

// ----- STRUCT -----
struct IRSignal {
    uint32_t data;
    int bits;
};

// ----- GLOBAL -----
extern std::vector<IRSignal> irSignalList;

// ----- FUNCTIONS -----
void IR_InitReceiver();
void IR_InitTransmitter();

bool IR_Receive(uint32_t &receivedData, int &bits);
void IR_Send(uint32_t data, int bits);

void IR_SaveSignal(uint32_t data, int bits);
void IR_LoadSignalsFromSD();
void IR_ReplaySignal(int index);

#endif
