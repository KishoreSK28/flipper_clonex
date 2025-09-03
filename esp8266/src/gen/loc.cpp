#include "loc.h"
#include <SoftwareSerial.h>

static const int RXPin = D5;   // GPS TX -> ESP RX
static const int TXPin = D6;   // GPS RX -> ESP TX (optional)
static const uint32_t GPSBaud = 9600;

SoftwareSerial gpsSerial(RXPin, TXPin);
TinyGPSPlus gps;

void initGPS() {
    gpsSerial.begin(GPSBaud);
    Serial.println("[GPS] Initialized");
}

bool readGPS(float &lat, float &lng) {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isUpdated()) {
        lat = gps.location.lat();
        lng = gps.location.lng();
        return true;
    }
    return false;
}
