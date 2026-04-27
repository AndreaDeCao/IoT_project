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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// ================= WIFI =================
const char* ssid = "HCM_NTW";
const char* password = "susamogus";
const char* DATA_URL = "https://iot-project-group-14.onrender.com/salva";


// ================= SENSORI =================
Adafruit_VCNL4010 vcnl;


#define SOGLIA_PROX 2400


// ================= IR =================
#define IR_SENSOR1 18
#define IR_SENSOR2 19


volatile bool passaggio1 = false;
volatile bool passaggio2 = false;


volatile unsigned long tempo1 = 0;
volatile unsigned long tempo2 = 0;


// ================= PARAMETRI =================
float distanza12_cm = 20.0;
float sogliaVelocita = 50.0;


// ================= FSM =================
typedef enum {
  WAIT,
  IR1,
  IR2,
  COMPUTATION
} State;

State current_state = WAIT;

// ================= ISR =================
void IRAM_ATTR ISR_SENSOR1() {
  passaggio1 = true;
}

void IRAM_ATTR ISR_SENSOR2() {
  passaggio2 = true;
}

// ================= WIFI =================
bool connectWiFi() {
  WiFi.disconnect(true); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long start = millis();

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\n Successfully Connected!");

  return WiFi.status() == WL_CONNECTED;
}


// ================= SEND DATA =================
void send_RESULT(float speed, bool triggered) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(15000);
  http.begin(client, DATA_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"speed\":" + String(speed, 2) + ",";
  payload += "\"triggered\":" + String(triggered ? "true" : "false");
  payload += "}";

  Serial.println("Invio payload: " + payload);
  int code = http.POST(payload);

  if (code > 0) {
    Serial.print("HTTP: ");
    Serial.println(code);
    Serial.println(http.getString());
  } else {
    Serial.print("POST fallito: ");
    Serial.println(HTTPClient::errorToString(code));
  }

  http.end();
}


// ================= STATI =================


void fn_WAIT() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WAIT");
  display.display();
  Serial.println("WAIT STATUS");

  uint16_t prox = vcnl.readProximity();
  Serial.print("Proximity: ");
  Serial.println(prox);


  if (prox > SOGLIA_PROX) {
    Serial.println("VCNL Trigger!");
    passaggio1 = true;
    current_state = IR1;
  }
}


void fn_IR1() {
  if (!passaggio1) return;

  tempo1 = micros();
  Serial.println("BAR1");


  display.clearDisplay();
  display.setCursor(0,0);
  display.println("BAR1 OK");
  display.display();

  passaggio2 = false;
  current_state = IR2;
}


void fn_IR2() {
  delay(500);
  tempo2 = micros();
  Serial.println("BAR2");
  passaggio2 = true;
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("BAR2 OK");
  display.display();

  current_state = COMPUTATION;
}


void fn_COMPUTATION() {
  Serial.println("COMPUTATION");
  noInterrupts();
  unsigned long t1 = tempo1;
  unsigned long t2 = tempo2;
  interrupts();


  float t = (t2 - t1) / 1000000.0;
  float speed = distanza12_cm / t;
  bool triggered = speed > sogliaVelocita;


  Serial.print("Velocita: ");
  Serial.println(speed);


  display.clearDisplay();
  display.setCursor(0,0);


  if (triggered) {
    display.println("OVER SPEED!");
  } else {
    display.println("OK");
  }


  display.display();


  send_RESULT(speed, triggered);


  // reset
  tempo1 = 0;
  tempo2 = 0;


  current_state = WAIT;
}


// ================= SETUP =================
void setup() {
  Serial.begin(115200);


  Wire.begin(21,22);
  Wire.setClock(100000);


  if (!vcnl.begin()) {
    Serial.println("VCNL4010 non trovato!");
    while (1);
  }


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display non trovato");
    while (1);
  }


  pinMode(IR_SENSOR1, INPUT_PULLUP);
  pinMode(IR_SENSOR2, INPUT_PULLUP);


  attachInterrupt(IR_SENSOR1, ISR_SENSOR1, FALLING);
  attachInterrupt(IR_SENSOR2, ISR_SENSOR2, FALLING);


  connectWiFi();


  Serial.println("Sistema pronto");
}


// ================= LOOP =================
void loop() {
  switch (current_state) {
    case WAIT: fn_WAIT(); break;
    case IR1: fn_IR1(); break;
    case IR2: fn_IR2(); break;
    case COMPUTATION: fn_COMPUTATION(); break;
  }


  delay(50); // piccolo respiro CPU
}

