#include <SPI.h>
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
#include <nRF24L01.h>
#include <RF24.h>
#include "I2Cdev.h"


RF24 radio(7, 8);  // CE, CSN 
bool dmpReady = false; 

const byte address[6] = "00001"; 
struct DataPacket {
  byte x;
  bytet y;
  byte z;
};
DataPacket data;
uint8_t fifoBuffer[64]; // FIFO storage buffer
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
MPU6050 mpu;



void setupMPU(){
  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties and set 100Khz
      
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  mpu.initialize();
  int devStatus = mpu.dmpInitialize();
  // make sure it worked (returns 0 if so)
  if (devStatus == 0) 
  {
      // Calibration Time: generate offsets and calibrate our MPU6050
      mpu.CalibrateAccel(6);
      mpu.CalibrateGyro(6);
      mpu.setDMPEnabled(true);
      dmpReady = true;
  }

}

void setup() {
  Serial.begin(9600);
  Serial.println("=== TRASMETTITORE ===");
  // Svegliamo l'MPU
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
  
  Serial.println("Setup completato!");
  setupMPU();  
}

void loop() {
  if (!dmpReady) return; // se dmpReady è falso non è andata bene il set up
 // 3. Leggiamo solo se i dati sono pronti nel buffer
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { 
   mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    int xAxisValue = constrain(ypr[2] * 180/M_PI, -90, 90);
    int yAxisValue = constrain(ypr[1] * 180/M_PI, -90, 90);
    int zAxisValue = constrain(ypr[0] * 180/M_PI, -90, 90);    
    data.x = map(xAxisValue, -90, 90, 0, 254); 
    data.y= map(yAxisValue, -90, 90, 0, 254);
    data.z = map(zAxisValue, -90, 90, 0, 254);    
  }


  // 4. Invio via radio
  bool ok=radio.write(&data, sizeof(data));

  // Debug seriale
  Serial.print("X = "); Serial.print(data.x);
  Serial.print(" | Y = "); Serial.print(data.y);
  Serial.print(" | Z = "); Serial.println(data.z);

  delay(50); // Un piccolo delay aiuta la stabilità della radio
  if(ok){
    Serial.println("inviato comnado di guida");
  }
  else{
    Serial.println("errore di comunicazione");
  }
}
