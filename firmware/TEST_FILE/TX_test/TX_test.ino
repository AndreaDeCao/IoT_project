#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);   // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  Serial.println("=== TRASMETTITORE ===");

  if (!radio.begin()) {
    Serial.println("Modulo NRF24 NON trovato!");
    while (1);
  }

  radio.setChannel(108);              // Canale lontano dal WiFi
  radio.setDataRate(RF24_250KBPS);    // Più stabile
  radio.setPALevel(RF24_PA_MIN);      // Potenza bassa per test
  radio.setRetries(5, 15);

  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Trasmettitore pronto!");
}

void loop() {
  int numero = 42;

  bool ok = radio.write(&numero, sizeof(numero));

  if (ok) {
    Serial.println("Inviato: 42");
  } else {
    Serial.println("Errore invio!");
  }

  delay(1000);
}

