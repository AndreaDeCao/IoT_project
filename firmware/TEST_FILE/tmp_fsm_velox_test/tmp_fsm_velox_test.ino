#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VCNL4010.h"
#include "velox.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <PubSubClient.h>

// ================= VCNL4010 =================
Adafruit_VCNL4010 BAR1;
Adafruit_VCNL4010 BAR2;
TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);

// ================= DISPLAY =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_1, OLED_RESET);

// ================= PIN I2C =================
#define I2C1_SDA 21
#define I2C2_SDA 18
#define I2C1_SCL 22
#define I2C2_SCL 19

// ================= WIFI =================
const char* ssid = "HCM.NTW";
const char* password = "susamogus";
const char* DATA_URL = "https://iot-project-group-14.onrender.com/salva";

// ================= VARIABILI =================
unsigned long tempo1 = 0;
unsigned long tempo2 = 0;
float currentSpeed = 0.0;
bool overSpeed = false;

// ================= PARAMETRI =================
#define SOGLIA_PROX 2300
#define SOGLIA_PROX_1 2300
#define SOGLIA_PROX_2 2400
#define NUM_LETTURE_STABILI 3

extern const float distanza12_cm = 4.0; // distance between two transimtters
extern const float sogliaVelocita = 2.5; // m/s

// ================= FSM =================
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

// ================= WEB CONFIGURATION=================

void scanWiFi() {
  Serial.println("Scanning WiFi networks...");

  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("Nessuna rete trovata");
  } else {
    Serial.print("Reti trovate: ");
    Serial.println(n);

    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (RSSI ");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm)");

      if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
        Serial.print(" [OPEN]");
      else
        Serial.print(" [PROTECTED]");

      Serial.println();
    }
  }

  Serial.println("Scan completata");
}


bool initWiFi() {
  WiFi.mode(WIFI_STA);
  if (WiFi.status() == WL_CONNECTED) return true;

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    if (millis() - start > 15000){
      Serial.println("\n[!] Error: it was impossible to connect to the network. Timeout.");
      return false;
    }
  }

  Serial.println("\nSuccessfully Connected to the network!");
  Serial.print("Assigned IP to the board: ");
  Serial.println(WiFi.localIP());
  return true;
}

// ================= JSON HANDLING =================
void performPostRequest() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  if (http.begin(client, DATA_URL)) {
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"speed\":" + String(currentSpeed, 2) +
                   ",\"sensorTriggered\":" + String(overSpeed ? "true" : "false") + "}";
    char jsonBuffer[128];
    snprintf(jsonBuffer, sizeof(jsonBuffer), "%s", payload.c_str());

    int httpResponseCode = http.POST(jsonBuffer);

    if (httpResponseCode < 0) {
      Serial.printf("Errore: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }
}

// ================= SETUP =================

void setup() {
  Serial.begin(115200);

  // I2C
  I2C_1.begin(I2C1_SDA, I2C1_SCL, 100000); //beginning the two IC2 connection on the predeterminated ports
  I2C_2.begin(I2C2_SDA, I2C2_SCL, 100000);

  // VCNL4010 and display initialization

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[!] Error: Display couldn't be initialized.");
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Ready");
  display.display();

  if (!BAR1.begin(VCNL4010_I2CADDR_DEFAULT, &I2C_1)) {
    Serial.println("[!] Error: First proximity sensor not detected, please check the cables");
    while (1);
  }

  if (!BAR2.begin(VCNL4010_I2CADDR_DEFAULT, &I2C_2)) {
    Serial.println("[!] Error: Second proximity sensor, please check the cables");
    while (1);
  }
  //overclocking the IR proximity sensor current
  BAR1.setLEDcurrent(200);
  BAR2.setLEDcurrent(200);
  BAR1.setFrequency(VCNL4010_3_90625);
  BAR2.setFrequency(VCNL4010_3_90625);

  //Serial printing and both VCNL test
  Serial.println("Sensors have been correctly initialized");
  Serial.print("BAR1 test: ");
  Serial.println(BAR1.readProximity());
  Serial.print("BAR2 test: ");
  Serial.println(BAR2.readProximity());

  // WIFI INITIALIZATION
  scanWiFi();
  initWiFi();
  Serial.println("AUTOVELOX HAS BEEN CORRECTLY INITIALIZED -> LOOP PHASE");
  delay(1000);
}

// ================= LOOP =================
void loop() {
  if (current_state < NUM_STATES) {
    fsm[current_state].state_function();
  }
  delayMicroseconds(500);
}

// ================= FSM FUNZIONI =================

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

// ================= PRIMA BARRIERA =================
void fn_BAR1() {
  Serial.println("Attesa barriera 1");
  while (BAR1.readProximity() < SOGLIA_PROX_1) {
  }
  tempo1 = micros();
  Serial.println("BAR1 triggerata");
  current_state = IR2_STATE;
}

// ================= SECONDA BARRIERA =================
void fn_BAR2() {
  Serial.println("Attesa barriera 2");

  while (BAR2.readProximity() < SOGLIA_PROX_2) {
  }
  tempo2 = micros();
  Serial.println("BAR2 triggerata");
  current_state = COMPUTATION;
}

// ================= CALCOLO =================
void fn_RESULT() {
  float t_secondi = (float)(tempo2 - tempo1) / 1000000.0;

  if (t_secondi <= 0.001) {
    Serial.println("Errore tempo");
    current_state = WAIT;
    return;
  }

  //speed calculation and status setting
  currentSpeed = (distanza12_cm / t_secondi) / 100.0; // in m/s
  overSpeed = (currentSpeed > sogliaVelocita);

  Serial.print("Tempo: ");
  Serial.println(t_secondi, 6);
  Serial.print("Velocita: ");
  Serial.print(currentSpeed, 2);
  Serial.println(" m/s");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(overSpeed ? "*** OVER SPEED ***" : "VELOCITA OK");
  display.print("Vel: ");
  display.print(currentSpeed);
  display.println(" m/s");
  display.display(); 

  //sending data to the webServer
  performPostRequest();
  // time milestones resetted
  tempo1 = 0;
  tempo2 = 0;

  current_state = WAIT;
}