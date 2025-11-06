//#include <Arduino.h>
//#include <IRremote.h>
#include <libraries/IRremote.h>

// --- Pin ---
const int IR_LEDS_TRANS[3] = {9, 10, 11};  // LED1, LED2, LED3
const int IR_SENSOR1 = 3;
const int IR_SENSOR2 = 4;
const int IR_SENSOR3 = 5;

// Parametri
const float distanza12_cm = 10.0;
const float sogliaVelocita = 50.0; // cm/s

// Variabili globali
volatile unsigned long tempo1 = 0;
volatile unsigned long tempo2 = 0;
volatile bool passaggio1 = false;
volatile bool passaggio2 = false;

// IR Remote ESP32
const int CARRIER_FREQ = 38;  // kHz
IRsend irsend1(IR_LEDS_TRANS[0]);
IRsend irsend2(IR_LEDS_TRANS[1]);
IRsend irsend3(IR_LEDS_TRANS[2]);  // trasmettitore per allerta velocità

uint16_t rawData[] = {500, 500, 500, 500};

// Interrupt sensori
void sensore1Attivato() {
  if (!passaggio1) {
    tempo1 = micros();
    passaggio1 = true;
  }
}

void sensore2Attivato() {
  if (passaggio1 && !passaggio2) {
    tempo2 = micros();
    passaggio2 = true;
  }
}

void setup() {
  Serial.begin(9600);

  // Pin trasmettitori
  for (int i = 0; i < 3; i++) pinMode(IR_LEDS_TRANS[i], OUTPUT);

  // Pin sensori
  pinMode(IR_SENSOR1, INPUT);
  pinMode(IR_SENSOR2, INPUT);
  pinMode(IR_SENSOR3, INPUT);

  // Interrupt sensori 1 e 2
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR1), sensore1Attivato, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR2), sensore2Attivato, FALLING);

  // Inizializza IRsend
  irsend1.begin(IR_LEDS_TRANS[0]);
  irsend2.begin(IR_LEDS_TRANS[1]);
  irsend3.begin(IR_LEDS_TRANS[2]);

  Serial.println("Sistema pronto.");
}

void loop() {
  // Modulazione continua LED1 e LED2
  irsend1.sendRaw(rawData, sizeof(rawData)/sizeof(rawData[0]), CARRIER_FREQ);
  irsend2.sendRaw(rawData, sizeof(rawData)/sizeof(rawData[0]), CARRIER_FREQ);

  // Misura velocità
  if (passaggio2) {
    noInterrupts();
    unsigned long t1 = tempo1;
    unsigned long t2 = tempo2;
    interrupts();

    float t_secondi = (t2 - t1) / 1000000.0;
    float velocita12 = distanza12_cm / t_secondi; // cm/s
    Serial.print("Velocità 1->2: "); Serial.println(velocita12);

    // --- Se velocità supera soglia, trasmetti con LED3 ---
    if (velocita12 > sogliaVelocita) {
      irsend3.sendRaw(rawData, sizeof(rawData)/sizeof(rawData[0]), CARRIER_FREQ);
      Serial.println("Allerta: velocità alta! LED3 trasmette.");
    } else {
      digitalWrite(IR_LEDS_TRANS[2], LOW); // LED3 spento se velocità ok
    }

    passaggio1 = passaggio2 = false;
  }

  // Lettura terzo ricevitore
  int stato3 = digitalRead(IR_SENSOR3);
  if (stato3 == LOW) Serial.println("Ricevitore 3 rileva fascio allerta!");

  delay(1);
}