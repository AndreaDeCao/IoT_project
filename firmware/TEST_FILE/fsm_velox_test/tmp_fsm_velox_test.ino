#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VCNL4010.h"
#include "velox.h"
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
// ⚠️⚠️⚠️ CODE NEEDS TO BE TESTED WITH INTERRUPTS, WILL IMPLEMENT STE'S CODE'S LOGIC
// ---------------- DISPLAY ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- VCNL4010 ----------------
Adafruit_VCNL4010 IR1;
Adafruit_VCNL4010 IR2;
TwoWire I2C_1 = TwoWire(0); //IC2 setting for dual connection to the sensors
TwoWire I2C_2 = TwoWire(1);
#define SOGLIA 2400   // analogic value for VCNL'S trigger

// ---------------- WIFI ----------------
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "Velox-module";
const char* password = ""; //⚠️⚠️⚠️ beware, compile null fields
const char* hostssid = ""; //⚠️⚠️⚠️ 
const char* hostpassword = ""; //⚠️⚠️⚠️

// ---------------- MQTT BROKER VARIABLES -----------------
const char* mqtt_server = "" //⚠️⚠️⚠️ Insert IP address of the computer which is hosting the mqtt broker
 
// ---------------- VARIABILI ----------------
unsigned long tempo1 = 0;
unsigned long tempo2 = 0;

float currentSpeed = 0.0;
bool sensorTriggered = false;

// ---------------- FSM ----------------
typedef enum {
  WAIT,
  IR1_STATE,
  IR2_STATE,
  COMPUTATION,
  NUM_STATES
} State_v;

typedef struct {
  State_v state;
  void (*state_function)(void);
} StateMachine_t;

// forward declarations
void fn_START();
void fn_BAR1();
void fn_BAR2();
void fn_RESULT();

StateMachine_t fsm[] = {
  {WAIT, fn_START},
  {IR1_STATE, fn_BAR1},
  {IR2_STATE, fn_BAR2},
  {COMPUTATION, fn_RESULT}
};

State_v current_state = WAIT;

// ---------------- WEB ----------------
void handleData() {
  String json = "{";
  json += "\"speed\":" + String(currentSpeed, 2) + ",";
  json += "\"triggered\":" + String(sensorTriggered ? "true" : "false");
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.send(200, "text/html",
  "<h1>Speed Camera</h1>"
  "<p>Speed: <span id='speed'>0</span> | "
  "<span id='trig'>OK</span></p>"
  "<script>setInterval(()=>fetch('/data').then(r=>r.json()).then(d=>{"
  "document.getElementById('speed').textContent=d.speed;"
  "document.getElementById('trig').textContent=d.triggered?'Too fast':'OK';"
  "}),1000);</script>");
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);

  // I2C
  I2C_1.begin(I2C1_SDA, I2C_SCL, 100000); //beginning the two IC2 connection on the predeterminated ports
  I2C_2.begin(I2C2_SDA, I2C_SCL, 100000);

  // Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED non trovato!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Ready");
  display.display();

  // VCNL4010 initialization
  if (!IR1.begin()) {
    Serial.println("First proximity sensor not detected, please check the cables");
    while (1);
  } else if(!IR2.begin()) {
    Serial.println("Second proximity sensor, please check the cables");
    while(1);
  }
  //overclocking the IR proximity sensor current
  IR1.setLEDcurrent(200);
  IR2.setLEDcurrent(200);
  Serial.println("Sensors have been correctly initialized");

  // WiFi Accesspoint creation
  WiFi.softAP(ssid, password);
  Serial.println("AP IP: " + WiFi.softAPIP().toString());
  server.on("/data", handleData);
  server.on("/", handleRoot);
  server.begin();

  // Connection to Wifi network -> the MQTT broker should be the network host
  WiFi.begin(hostssid, hostpassword);
  Serial.print("Connecting")
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // MQTT broker connection initialization
  client.setServer(mqtt_server, 1883)
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();

  if (current_state < NUM_STATES) {
    fsm[current_state].state_function();
  }
}

// ---------------- FSM FUNZIONI ----------------

void fn_START() {
  Serial.println("WAIT...");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("WAIT");
  display.display();

  delay(500);

  current_state = IR1_STATE;
}

// -------- PRIMA BARRIERA --------
void fn_BAR1() {
  Serial.println("Attesa barriera 1");

  while (IR1.readProximity() < SOGLIA) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WAIT CAR 1");
    display.display();
    delay(50);
  }

  // doppio check anti-rumore
  delay(50);
  if (IR1.readProximity() < SOGLIA) return;

  tempo1 = micros();
  Serial.println("Auto rilevata (1)");

  display.clearDisplay();
  display.println("CAR 1!");
  display.display();
  delay(500);

  current_state = IR2_STATE;
}

// -------- SECONDA BARRIERA --------
void fn_BAR2() {
  Serial.println("Attesa barriera 2");

  while (IR2.readProximity() < SOGLIA) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WAIT CAR 2");
    display.display();
    delay(50);
  }

  delay(50);
  if (IR2.readProximity() < SOGLIA) return;

  tempo2 = micros();
  Serial.println("Auto rilevata (2)");

  display.clearDisplay();
  display.println("CAR 2!");
  display.display();
  delay(500);

  current_state = COMPUTATION;
}

// -------- CALCOLO --------
void fn_RESULT() {
  float t_secondi = (tempo2 - tempo1) / 1000000.0;

  if (t_secondi <= 0) {
    Serial.println("Errore tempo");
    current_state = WAIT;
    return;
  }
  //speed calculation and status setting
  currentSpeed = distanza12_cm / t_secondi;
  sensorTriggered = (currentSpeed > sogliaVelocita);
  Serial.print("Tempo: ");
  Serial.println(t_secondi, 6);

  Serial.print("Velocita: ");
  Serial.println(currentSpeed, 2);
  //⚠️⚠️⚠️ DISPLAY TESTING and LED FUNCTIONS TESTING NEEDED
  display.clearDisplay();
  display.setTextSize(1);

  if (sensorTriggered) {
    display.println("TOO FAST!");
  } else {
    display.println("OK");
  }

  display.print("Speed: ");
  display.println(currentSpeed);

  display.display();

  delay(2000);

  // time milestones resetted
  tempo1 = 0;
  tempo2 = 0;

  current_state = WAIT;
}