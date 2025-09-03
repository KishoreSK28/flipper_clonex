#ifndef LOC_H
#define LOC_H

#include <Arduino.h>
#include <TinyGPS++.h>

// Functions
void initGPS();
bool readGPS(float &lat, float &lng);

#endif
