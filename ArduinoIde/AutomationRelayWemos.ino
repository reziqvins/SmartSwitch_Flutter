#include <ESP8266WiFi.h>         // Use ESP32WiFi.h if using ESP32
#include <ESP8266WebServer.h>    // Use WebServer.h if using ESP32

#define RELAY_PIN D1   // Pin untuk relay
#define BUZZER_PIN D2  // Pin untuk buzzer
#define SWITCH_PIN D3  // Pin untuk switch

const char* ssid = "DARUSMAN HOME"; 
const char* password = "23698000"; 

ESP8266WebServer server(80); // Create a web server object that listens on port 80

bool relayState = false;        
bool lastSwitchState = LOW;     
bool currentSwitchState;        

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);      
  pinMode(BUZZER_PIN, OUTPUT);     
  pinMode(SWITCH_PIN, INPUT_PULLUP); 
  digitalWrite(RELAY_PIN, LOW);   

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  // Beep continuously until connected
  while (WiFi.status() != WL_CONNECTED) {
    tone(BUZZER_PIN, 1000, 100); // Beep
    delay(200);
    noTone(BUZZER_PIN); // Stop beeping
    delay(200);
    Serial.println("Connecting to WiFi...");
  }
  
  // Once connected, stop beeping and beep three times
  noTone(BUZZER_PIN);
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000, 100); // Beep
    delay(200);
    noTone(BUZZER_PIN); // Stop beeping
    delay(200);
  }

  Serial.println("Connected to WiFi");
  Serial.println("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  
  server.on("/relay/on", [](){
    digitalWrite(RELAY_PIN, HIGH);
    tone(BUZZER_PIN, 1000, 100); // Beep once
    server.send(200, "text/plain", "Relay ON");
  });

  server.on("/relay/off", [](){
    digitalWrite(RELAY_PIN, LOW);
    tone(BUZZER_PIN, 1000, 100); // Beep twice
    delay(200);
    tone(BUZZER_PIN, 1000, 100);
    server.send(200, "text/plain", "Relay OFF");
  });

  server.begin();
}

void loop() {
  server.handleClient(); // Handle incoming client requests

  currentSwitchState = digitalRead(SWITCH_PIN);

  // Detect if the switch is pressed
  if (currentSwitchState == LOW && lastSwitchState == HIGH) {
    relayState = !relayState;  // Toggle relay state

    if (relayState) {
      digitalWrite(RELAY_PIN, HIGH); // Turn on relay
      tone(BUZZER_PIN, 1000, 100);   // Beep once
    } else {
      digitalWrite(RELAY_PIN, LOW);  // Turn off relay
      tone(BUZZER_PIN, 1000, 100);   // Beep twice
      delay(200);
      tone(BUZZER_PIN, 1000, 100);
    }
  }

  lastSwitchState = currentSwitchState; 
  delay(50); // Debounce delay
}
