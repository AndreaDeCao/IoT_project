// // /*
// //   Parte per wifi PROVA!!!!!!!!!!!!!!!
// // */

// #include <WiFi.h>
// #include <WebServer.h>

// const char* ssid = "ESP32_SSID";          //temp
// const char* password = "ESP32_PASSWORD";  //temp

// WebServer server(80);

// // Variabili di esempio
// float speed = 12.5;
// bool led = true;
// float tempo1 = 0;
// // float tempo2??;    //tempo effettivo per arrivo alla seconda barriera

// //dati infrarosso quindi il tempo

// void handleData() {
//   String json = "{";
//   json += "\"speed\":" + String(speed, 2) + ",";
//   json += "\"led\":" + String(led ? "true" : "false");
//   json += "}";
//   server.send(200, "application/json", json);
// }

// void setup() {
//   Serial.begin(115200); //115200

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println(WiFi.localIP());

//   server.on("/data", handleData);
//   server.on("/", []() {
//     server.send(200, "text/html", "<h1>Speed Camera Dashboard</h1>");
//   });

//   server.begin();
//   Serial.println("HTTP server started");
// }

// void loop() {
//   server.handleClient();
// }

//versione con access point

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "test_test";
const char* password = "";

WebServer server(80);

// Speed camera var
float currentSpeed = 0.0;
bool sensorTriggered = false;
unsigned long sensor1Time = 0;
unsigned long sensor2Time = 0;
const float sensorDistance = 1.0; // Distance between sensors in m

void handleData() {
  server.sendHeader("Access-Control", "*");
  String json = "{";
  json += "\"speed\":" + String(currentSpeed, 2) + ",";
  json += "\"triggered\":" + String(sensorTriggered ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(9600);
  
  WiFi.softAP(ssid, password);
  Serial.println("Access Point created");
  Serial.println(WiFi.softAPIP());

  server.on("/data", handleData);
  server.on("/", []() {
    server.send(200, "text/html", 
      "<h1>Speed Camera</h1><p>Speed: <span id='speed'>0</span> km/h</p>");
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  
  // Add IR sensor's part, -> reading logic here
  // Calculate speed
}
