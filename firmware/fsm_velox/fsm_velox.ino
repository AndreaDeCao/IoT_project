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
#include <Preferences.h>

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

// ================= PARAMETRI =================
#define SOGLIA_OFFSET 50
#define LETTURE_CALIBRAZIONE 20
#define TIMEOUT_ATTESA_MS 3000
#define N_LETTURE_MEMORIA 5
#define distanza12_cm 4.0 // distance between two transimtters
#define sogliaVelocita 0.75 // m/s

// ================= VARIABILI =================
Preferences prefs;
float storicoVelocita[N_LETTURE_MEMORIA] = {0};
const char* PREFS_KEY = "storicov";
unsigned long tempo1 = 0;
unsigned long tempo2 = 0;
unsigned long tempo_inizio_attesa = 0;
float currentSpeed = 0.0;
bool overSpeed = false;
unsigned int SOGLIA_PROX_1 = 0;
unsigned int SOGLIA_PROX_2 = 0;


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

// ================= MEMORY SAVING ====================

  void saveHistoryInMemory() {
    prefs.begin("autovelox", false);  //opening stream in write mode
    prefs.putBytes(PREFS_KEY, (byte*)&storicoVelocita, sizeof(storicoVelocita));
    prefs.end();
    Serial.println("Speed was successfully saved in Flash Memory.");
  }

  void loadHistoryFromMemory(){
    prefs.begin("autovelox", true); //opening stream in read mode
    if (prefs.isKey(PREFS_KEY)) {
      prefs.getBytes(PREFS_KEY, (byte*)&storicoVelocita, sizeof(storicoVelocita));
      Serial.println("Speed History was correctly loaded");
    } else {
      Serial.println("[!] There wasn't any speed history loaded in memory.");
    }
    prefs.end();
  }

  void addSpeedToMemory(float speed){
    //speed values shift in the circular array -> least recent value is trashed
    for (int i = 0; i < N_LETTURE_MEMORIA - 1; i++) {
      storicoVelocita[i] = storicoVelocita[i + 1];
    }
    storicoVelocita[N_LETTURE_MEMORIA - 1] = speed;
    saveHistoryInMemory();
  }

  void printSpeedHistory() {
    bool hit = false;
    Serial.println("\nSPEED HISTORY => ");
    for (int i = 0; i < N_LETTURE_MEMORIA; i++) {
      if (storicoVelocita[i] > 0) {
        Serial.printf("[%d] %.2f m/s\n", i + 1, storicoVelocita[i]);
        hit = true;
      }
    }
    if (!hit) Serial.println("no History was found");
  }
  
// ================= VCNL CALIBRATION =================

void calibrateVCNL(){
  uint32_t proximity1 = 0;
  uint32_t proximity2 = 0;
  for (int i = 0; i < LETTURE_CALIBRAZIONE; i++){
    proximity1 += BAR1.readProximity();
    proximity2 += BAR2.readProximity();
    delay(20);
}

  SOGLIA_PROX_1 = (uint16_t)(proximity1 / LETTURE_CALIBRAZIONE) + SOGLIA_OFFSET;
  SOGLIA_PROX_2 = (uint16_t)(proximity2 / LETTURE_CALIBRAZIONE) + SOGLIA_OFFSET;

  Serial.printf("Proximity Threshold have been set -> BAR1: %u, BAR2: %u\n", SOGLIA_PROX_1, SOGLIA_PROX_2);
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
  Serial.print("BAR1 test: ");
  Serial.println(BAR1.readProximity());
  Serial.print("BAR2 test: ");
  Serial.println(BAR2.readProximity());
  calibrateVCNL();
  Serial.println("Sensors have been correctly initialized");

  // WIFI INITIALIZATION
  scanWiFi();
  initWiFi();
  Serial.println("AUTOVELOX HAS BEEN CORRECTLY INITIALIZED -> LOOP PHASE");
  delay(1000);

  // MEMORY SETUP
  loadHistoryFromMemory();
  printSpeedHistory();
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

  delayMicroseconds(500);
  Serial.println("Attesa barriera 1");
  current_state = IR1_STATE;
}

// ================= PRIMA BARRIERA =================
void fn_BAR1() {
  if (BAR1.readProximity() < SOGLIA_PROX_1) {
    return;
  }
  tempo1 = micros();
  tempo_inizio_attesa = millis();
  Serial.println("BAR1 triggerata");
  Serial.println("Attesa barriera 2");
  current_state = IR2_STATE;
}

// ================= SECONDA BARRIERA =================
void fn_BAR2() {
  if (BAR2.readProximity() < SOGLIA_PROX_2) {
    if(millis() - tempo_inizio_attesa > TIMEOUT_ATTESA_MS) {
      Serial.println("[!] Error, the vehicle was too slow -> returning to WAIT state");
      delay(1000);
      current_state = WAIT;
      tempo1 = 0;
    }
    return;
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
  //saving speed in memory 
  addSpeedToMemory(currentSpeed);
  //sending data to the webServer
  performPostRequest();
  // time milestones resetted
  tempo1 = 0;
  tempo2 = 0;

  current_state = WAIT;
}