#include <SPI.h>
#include <MPU6050_tockn.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

RF24 radio(7, 8);  // CE, CSN
const int MPU = 0x68; // Confermiamo 0x68 visto dallo scanner

const byte address[6] = "00001"; 
struct DataPacket {
  float x;
  float y;
  float z;
};
DataPacket data;

void setup() {
  Serial.begin(9600);
  Serial.println("=== TRASMETTITORE ===");
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
    if (!radio.begin()) {
    Serial.println("Modulo NRF24 NON trovato!");
    while (1);
  }
  
  // Svegliamo l'MPU
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);
  radio.setChannel(108);              // Canale lontano dal WiFi
  radio.setDataRate(RF24_250KBPS);    // Più stabile
  radio.setPALevel(RF24_PA_MIN);      // Potenza bassa per test
  radio.setRetries(5, 15);
  radio.openWritingPipe(address);
  radio.stopListening();           
  
  Serial.println("Setup completato!");
}

void loop() {
  // 1. Puntiamo al registro iniziale
  mpu6050.update();
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);

  // 2. CHIEDIAMO I DATI (Fondamentale!)
  // Chiediamo 6 byte (2 per X, 2 per Y, 2 per Z)
  Wire.requestFrom(MPU, 6, true); 
  unsigned long dt,t1;
  dt= millis() - t1;
  if (dt > 200) {
    t1=millis();
    float angleX = mpu6050.getAngleX();
    float angleY = mpu6050.getAngleY();
    Serial.println("=======================================================");
    data.x=angleX;
    data.y=angleY;
    Serial.println("x = "); Serial.print(data.x);
    Serial.println(" | y = "); Serial.print(data.y);
  if ((angleX) < 2 && (angleX)>-2 && (angleY)>-10&& (angleY)<10) {
      Serial.println("x = "); Serial.print(data.x);
      Serial.println(" | y = "); Serial.print(data.y);
      Serial.println("STOP");
  } 
  else{
    if((angleX) < 0 && (angleY)<-3){
      Serial.println("x = "); Serial.print(data.x);
      Serial.println(" | y = "); Serial.print(data.y); 
      Serial.println("indietro");
    }
    else {
      if((angleX) < 0 && (angleY)>30){
        Serial.println("x = "); Serial.print(data.x);
        Serial.println(" | y = "); Serial.print(data.y);
        Serial.println("Avanti");
    }
    }
    
  }
  if((angleX>19)&& (angleY)>-3){
    Serial.println("x = "); Serial.print(data.x);
    Serial.println(" | y = "); Serial.print(data.y);
    Serial.println("Destra");
  }
  else{
    if((angleX<-70)&& (angleY)>0.00){
      Serial.println("x = "); Serial.print(data.x);
      Serial.println(" | y = "); Serial.print(data.y);
      Serial.println("Sinistra");
    }

  }
  }

  // 3. Leggiamo solo se i dati sono pronti nel buffer
  /*if (Wire.available() >= 6) {
    data.x = mpu6050.getAngleX(); 
    data.y = mpu6050.getAngleY(); 
    data.z = mpu6050.getAngleZ(); 
  }
  */

  // 4. Invio via radio
  bool ok=radio.write(&data, sizeof(data));

  delay(50); // Un piccolo delay aiuta la stabilità della radio
  if(ok){
    Serial.println("inviato comnado di guida");
  }
  else{
    Serial.println("errore di comunicazione");
  }
}
