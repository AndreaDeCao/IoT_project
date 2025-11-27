// /*
//   Parte per wifi PROVA!!!!!!!!!!!!!!!
// */

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_SSID";          //temp
const char* password = "ESP32_PASSWORD";  //temp

WebServer server(80);

// Variabili di esempio
float speed = 12.5;
bool led = true;
float tempo1 = 0;
// float tempo2??;    //tempo effettivo per arrivo alla seconda barriera

//dati infrarosso quindi il tempo

void handleData() {
  String json = "{";
  json += "\"speed\":" + String(speed, 2) + ",";
  json += "\"led\":" + String(led ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200); //115200

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  server.on("/data", handleData);
  server.on("/", []() {
    server.send(200, "text/html", "<h1>Speed Camera Dashboard</h1>");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
