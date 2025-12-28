#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define RELAY_PIN D1
#define BUZZER_PIN D2
#define SWITCH_PIN D3
#define RESET_PIN D4  

ESP8266WebServer server(80);  
String webPage;              

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);  
  digitalWrite(RELAY_PIN, LOW);

  // Periksa apakah tombol reset ditekan
  if (digitalRead(RESET_PIN) == LOW) {
    
    resetEEPROM();
    Serial.println("WiFi credentials cleared. Restarting...");
    delay(2000);
    ESP.restart();
  }

  String savedSSID = readEEPROMString(0, 32);
  String savedPassword = readEEPROMString(32, 64);

  if (savedSSID != "" && savedPassword != "") {
    WiFi.mode(WIFI_AP_STA);  
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    Serial.println("Connecting to saved WiFi credentials...");

    /
    int timeout = 20;  // Timeout dalam 20 detik
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      delay(1000);
      Serial.print(".");
      timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(savedSSID);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      startWebServer();  
    } else {
      Serial.println("Failed to connect. Opening Access Point...");
      startAccessPoint();  
    }
  } else {
    startAccessPoint();  
  }
}

void loop() {
  server.handleClient();  

  
  if (digitalRead(RESET_PIN) == LOW) {
   
    resetEEPROM();
    Serial.println("WiFi credentials cleared. Restarting...");
    delay(2000);
    ESP.restart();
  }
}


void handleRoot() {
  server.send(200, "text/html", webPage);
}


void handleSaveCredentials() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    
    writeEEPROMString(0, ssid, 32);
    writeEEPROMString(32, password, 64);
    EEPROM.commit();

   
    String response = "<html><body><h1 style='color: #333; text-align: center;'>Saved! Rebooting and connecting to WiFi...</h1>";
    response += "<p style='text-align: center;'>Current IP address will be displayed after connection.</p>";
    response += "</body></html>";
    server.send(200, "text/html", response);
    delay(2000);
    ESP.restart();
  } else {
    server.send(200, "text/html", "<html><body><h1 style='color: red; text-align: center;'>Missing SSID or Password!</h1></body></html>");
  }
}


void startAccessPoint() {
  WiFi.mode(WIFI_AP);  
  WiFi.softAP("Darusman Home Automation");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  
  webPage = "<html><body style='font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center;'>";
  webPage += "<h1 style='color: #333;'>WiFi Setup</h1>";
  webPage += "<p>Connect to this device via Wi-Fi, and enter SSID and password.</p>";
  webPage += "<p>Current AP IP: <strong>" + IP.toString() + "</strong></p><br>";
  
  
  webPage += "<form action='/save' method='POST' style='background-color: #fff; padding: 20px; border-radius: 10px; display: inline-block;'>";
  webPage += "SSID: <input type='text' name='ssid' style='padding: 10px; width: 80%; margin-bottom: 10px;'><br>";
  webPage += "Password: <input type='password' name='password' style='padding: 10px; width: 80%; margin-bottom: 20px;'><br>";
  webPage += "<input type='submit' value='Save' style='padding: 10px 20px; background-color: #28a745; color: #fff; border: none; border-radius: 5px; cursor: pointer;'>";
  webPage += "</form><br><br>";
  
  
  webPage += "<a href='/detail'><button style='padding: 10px 20px; background-color: #007bff; color: #fff; border: none; border-radius: 5px; cursor: pointer;'>View Connection Details</button></a>";
  webPage += "</body></html>";

  
  server.on("/", handleRoot);
  server.on("/save", handleSaveCredentials);
  server.on("/detail", handleDetailPage);
  server.begin();  
  Serial.println("Access Point started and web server running");
}


void startWebServer() {
  IPAddress ip = WiFi.localIP();
  
  
  webPage = "<html><body style='font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center;'>";
  webPage += "<h1 style='color: #333;'>ESP8266 Connected</h1>";
  webPage += "<p>Connected to WiFi.</p>";
  webPage += "<p>Your IP Address is: <strong>" + ip.toString() + "</strong></p>";
  webPage += "</body></html>";

  
  server.on("/", handleRoot);
  server.begin();  // Start the server
  Serial.println("Web server started and connected to WiFi");
}


void handleDetailPage() {
  IPAddress ip = WiFi.localIP();
  String detailsPage = "<html><body style='font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center;'>";
  detailsPage += "<h1 style='color: #333;'>Connection Details</h1>";
  detailsPage += "<p>Connected to WiFi.</p>";
  detailsPage += "<p>Your IP Address is: <strong>" + ip.toString() + "</strong></p>";
  detailsPage += "</body></html>";
  server.send(200, "text/html", detailsPage);
}


void resetEEPROM() {
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);  
  }
  EEPROM.commit();
  Serial.println("EEPROM cleared!");
}

// Read a string from EEPROM
String readEEPROMString(int start, int len) {
  char data[len + 1];
  for (int i = 0; i < len; i++) {
    data[i] = EEPROM.read(start + i);
  }
  data[len] = '\0';  // Null-terminate string
  return String(data);
}

// Write a string to EEPROM
void writeEEPROMString(int start, String data, int len) {
  for (int i = 0; i < len; i++) {
    if (i < data.length()) {
      EEPROM.write(start + i, data[i]);
    } else {
      EEPROM.write(start + i, 0);
    }
  }
}

