#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);   // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  Serial.println("=== RICEVITORE ===");

  if (!radio.begin()) {
    Serial.println("Modulo NRF24 NON trovato!");
    while (1);
  }

  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setRetries(5, 15);

  radio.openReadingPipe(0, address);
  radio.startListening();

  Serial.println("Ricevitore pronto!");
}

void loop() {
  if (radio.available()) {
    int ricevuto;
    radio.read(&ricevuto, sizeof(ricevuto));

    Serial.print("Ricevuto: ");
    Serial.println(ricevuto);
  }
}

