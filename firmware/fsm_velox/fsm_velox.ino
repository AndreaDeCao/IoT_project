#include <Wire.h>
#include <Adafruit_VCNL4010.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ================= DISPLAY =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ================= PIN I2C =================
#define I2C1_SDA 22
#define I2C2_SDA 18
#define I2C1_SCL 23
#define I2C2_SCL 19

// ================= WIFI =================
const char* ssid = "HCM.NTW";
const char* password = "susamogus";
const char* DATA_URL = "https://iot-project-group-14.onrender.com/salva";

// ================= SENSORI + DISPLAY =================
Adafruit_VCNL4010 BAR1;
Adafruit_VCNL4010 BAR2;
TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_1, -1);

// ================= PARAMETRI =================
#define SOGLIA_PROX 2300
#define SOGLIA_PROX_1 2300
#define SOGLIA_PROX_2 2400
#define TIMEOUT_ATTESA_US 1000000   // NON USATO (disabilitato)
#define NUM_LETTURE_STABILI 3

float distanza12_cm = 20.0;
float sogliaVelocita = 50.0;

// ================= FSM =================
typedef enum {
  ATTESA_BAR1,
  ATTESA_BAR2
} State;

State current_state = ATTESA_BAR1;

unsigned long tempo_bar1_us = 0;
unsigned long tempo_inizio_attesa_us = 0;

// ================= LETTURA ROBUSTA =================
uint16_t leggiProximitaRobusta(Adafruit_VCNL4010 &sensore, TwoWire &bus, int tentativi = 3) {
  for (int i = 0; i < tentativi; i++) {
    uint16_t valore = sensore.readProximity();
    if (valore > 10 && valore < 5000) {
      return valore;
    }
    delay(2);
  }
  return 0;
}

bool isTriggerato(Adafruit_VCNL4010 &sensore, TwoWire &bus) {
  int conteggio = 0;
  for (int i = 0; i < NUM_LETTURE_STABILI; i++) {
    uint16_t valore = leggiProximitaRobusta(sensore, bus, 2);
    if (valore > SOGLIA_PROX) conteggio++;
    delay(1);
  }
  return (conteggio >= NUM_LETTURE_STABILI - 1);
}

// ================= WIFI =================
bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
    if (millis() - start > 15000) return false;
  }

  Serial.println("\nConnected!");
  return true;
}

void send_RESULT(float speed, bool over_speed) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  http.begin(client, DATA_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"speed\":" + String(speed, 2) +
                   ",\"sensorTriggered\":" + String(over_speed ? "true" : "false") + "}";

  http.POST(payload);
  http.end();
}

// ================= STATI =================

void stato_ATTESA_BAR1() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AUTOVELOX");
  display.println("IN ATTESA");
  display.display();

  uint16_t prox1 = leggiProximitaRobusta(BAR1, I2C_1);

  Serial.print("BAR1: ");
  Serial.println(prox1);

  if (prox1 > SOGLIA_PROX_1 && isTriggerato(BAR1, I2C_1)) {

    tempo_bar1_us = micros();
    tempo_inizio_attesa_us = micros();

    // 🔥 FIX: aspetta che BAR2 sia libero prima di iniziare
    while (leggiProximitaRobusta(BAR2, I2C_2) > SOGLIA_PROX_2) {
      delay(1);
    }

    current_state = ATTESA_BAR2;

    Serial.println("*** AUTO RILEVATO - ATTESA BAR2 ***");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AUTO RILEVATO!");
    display.println("Attendo BAR2...");
    display.display();
  }
}

void stato_ATTESA_BAR2() {

  uint16_t prox2 = leggiProximitaRobusta(BAR2, I2C_2);

  Serial.print("BAR2: ");
  Serial.println(prox2);

  if (prox2 > SOGLIA_PROX_2 && isTriggerato(BAR2, I2C_2)) {

    unsigned long tempo_bar2_us = micros();

    float delta_t_sec = (tempo_bar2_us - tempo_bar1_us) / 1000000.0;

    if (delta_t_sec > 0) {
      float velocita_cm_s = distanza12_cm / delta_t_sec;
      float velocita_m_s = velocita_cm_s / 100;
      bool over_speed = (velocita_m_s > sogliaVelocita);

      Serial.print("Tempo: ");
      Serial.print(delta_t_sec);
      Serial.print(" s  Velocita: ");
      Serial.print(velocita_m_s);
      Serial.println(" m/s");

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(over_speed ? "*** OVER SPEED ***" : "VELOCITA OK");
      display.print("Vel: ");
      display.print(velocita_m_s);
      display.println(" m/s");
      display.display();

      send_RESULT(velocita_m_s, over_speed);

      delay(2000);
    }

    current_state = ATTESA_BAR1;
  }

  // aggiornamento display non bloccante
  static unsigned long last_display = 0;
  if (millis() - last_display > 200) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ATTESA BAR2...");
    display.print("Tempo: ");
    display.print((micros() - tempo_bar1_us) / 1000);
    display.println(" ms");
    display.display();
    last_display = millis();
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  I2C_1.begin(I2C1_SDA, I2C1_SCL, 100000);
  I2C_2.begin(I2C2_SDA, I2C2_SCL, 100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display non trovato");
  }

  if (!BAR1.begin(VCNL4010_I2CADDR_DEFAULT, &I2C_1)) {
    Serial.println("BAR1 non trovato!");
    while (1);
  }

  if (!BAR2.begin(VCNL4010_I2CADDR_DEFAULT, &I2C_2)) {
    Serial.println("BAR2 non trovato!");
    while (1);
  }

  BAR1.setLEDcurrent(20);
  BAR2.setLEDcurrent(20);
  BAR1.setFrequency(VCNL4010_3_90625);
  BAR2.setFrequency(VCNL4010_3_90625);
  Serial.print("BAR1 test: ");
  Serial.println(BAR1.readProximity());
  Serial.print("BAR2 test: ");
  Serial.println(BAR2.readProximity());

  connectWiFi();

  Serial.println("AUTOVELOX PRONTO (NO TIMEOUT)");
}

// ================= LOOP =================
void loop() {
  switch (current_state) {
    case ATTESA_BAR1:
      stato_ATTESA_BAR1();
      break;

    case ATTESA_BAR2:
      stato_ATTESA_BAR2();
      break;
  }

  delayMicroseconds(500); // loop veloce
}