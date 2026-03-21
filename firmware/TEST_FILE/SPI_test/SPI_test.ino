#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/* 
1. nRF24L01.h (The Chip's "Grammar")

This library is a header file containing constants and definitions. It does not contain active commands; instead, it provides human-readable names for the internal memory addresses of the radio chip.

What it does: It defines the names of the internal registers (e.g., CONFIG, EN_AA, RF_CH).

Without it: You would have to use complex hexadecimal numbers to configure the radio (e.g., writing 0x05 instead of RF_CH).

Analogy: It is like a vocabulary or a dictionary that explains to Arduino the names of the internal "buttons" of the radio.

2. RF24.h (The Active "Instruction Manual")

This is the main library  that contains the high-level functions used in your code, such as radio.begin(), radio.write(), and radio.startListening().

What it does: It manages complex logic. When you call radio.begin(), this library executes hundreds of lines of background code to coordinate the SPI pins and power up the module.

Without it: You would have to manually program the entire sequence of electrical signals (bit by bit) just to send a single piece of data.

Analogy: It is like the driver who knows how to operate the car; you only give the simple commands like "start" or "turn."
*/

RF24 radio(7,8); // creation of radio object

void setup() {
  Serial.begin(9600);
}

void loop() {
    if (radio.begin()) {  // IF TRUE ARDUINO FOUND THE NRF24L101 RADIO
    Serial.println("Module NRF found!");
  } else {
    Serial.println("Modulo NRF not found!");
  }
}
