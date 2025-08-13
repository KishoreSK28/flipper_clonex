#include "server.h"
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <./globals.h>
#include<SD.h>
#include<SPI.h>
void storageInfo(WebServer &server);
#define SD_CS   25
#define SD_MOSI 32
#define SD_MISO 34
#define SD_SCK  33

// Helper
bool isSDReady = false;

void storageInfo(WebServer &server) {
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  String json = "{";
  json += "\"SPIFFS_total\":" + String(totalBytes) + ",";
  json += "\"SPIFFS_used\":" + String(usedBytes) + ",";
  json += "\"SPIFFS_free\":" + String(totalBytes - usedBytes) + ",";

  if (isSDReady) {
    uint64_t sdSize = SD.totalBytes();
    uint64_t sdUsed = SD.usedBytes();  // Only on some boards
    json += "\"SD_total\":" + String(sdSize) + ",";
    json += "\"SD_used\":" + String(sdUsed) + ",";
    json += "\"SD_free\":" + String(sdSize - sdUsed);
  }

  json += "}";
  server.send(200, "application/json", json);
}

void listFiles(WebServer &server) {
  String response = "{\"SPIFFS\":[";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    response += "\"" + String(file.name()) + "\"";
    file = root.openNextFile();
    if (file) response += ",";
  }
  response += "]";

  if (isSDReady) {
    response += ",\"SD\":[";
    File sdFile = SD.open("/");
    File f = sdFile.openNextFile();
    while (f) {
      response += "\"" + String(f.name()) + "\"";
      f = sdFile.openNextFile();
      if (f) response += ",";
    }
    response += "]";
  }

  response += "}";
  server.send(200, "application/json", response);
}

void uploadFile(WebServer &server) {
  static File fsUploadFile;
  HTTPUpload& upload = server.upload();
  String targetFS = server.arg("fs"); // "spiffs" or "sd"

  if (upload.status == UPLOAD_FILE_START) {
    String path = "/" + upload.filename;
    if (targetFS == "sd" && isSDReady) {
      fsUploadFile = SD.open(path, FILE_WRITE);
    } else {
      fsUploadFile = SPIFFS.open(path, FILE_WRITE);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) fsUploadFile.close();
    server.send(200, "text/plain", "File uploaded");
  }
}

void deleteFile(WebServer &server) {
  if (!server.hasArg("filename") || !server.hasArg("fs")) {
    server.send(400, "text/plain", "Missing 'filename' or 'fs'");
    return;
  }

  String filename = "/" + server.arg("filename");
  String targetFS = server.arg("fs");

  bool deleted = false;
  if (targetFS == "sd" && isSDReady) {
    deleted = SD.remove(filename);
  } else {
    deleted = SPIFFS.remove(filename);
  }

  if (deleted) {
    server.send(200, "text/plain", "File deleted");
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

void downloadFile(WebServer &server) {
  if (!server.hasArg("filename") || !server.hasArg("fs")) {
    server.send(400, "text/plain", "Missing 'filename' or 'fs'");
    return;
  }

  String filename = "/" + server.arg("filename");
  String targetFS = server.arg("fs");

  File file;
  if (targetFS == "sd" && isSDReady) {
    file = SD.open(filename, FILE_READ);
  } else {
    file = SPIFFS.open(filename, FILE_READ);
  }

  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  server.streamFile(file, "application/octet-stream");
  file.close();
}

void startStorageServer(WebServer &server) {
  Serial.println("Initializing SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }

  Serial.println("Initializing SD card...");
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (SD.begin(SD_CS)) {
    Serial.println("SD card initialized.");
    isSDReady = true;
  } else {
    Serial.println("SD card mount failed.");
  }

  // Endpoints
  server.on("/storageinfo", HTTP_GET, [&]() { storageInfo(server); });
  server.on("/list", HTTP_GET, [&]() { listFiles(server); });
  server.on("/upload", HTTP_POST, [&]() { server.send(200); },
    [&]() { uploadFile(server); });
  server.on("/delete", HTTP_POST, [&]() { deleteFile(server); });
  server.on("/download", HTTP_GET, [&]() { downloadFile(server); });

  Serial.println("Storage server ready.");
}
