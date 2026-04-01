#include <SPI.h>                          // Include the library for the SPI comunication useb by NRF24
#include "MPU6050_6Axis_MotionApps20.h"   // Include the advance functions for processing quaternions using DMP
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE // In IC2dev.h in the row riga 61 #define I2CDEV_IMPLEMENTATION       I2CDEV_ARDUINO_WIRE
    #include "Wire.h"                     // Include I2C comunication library
#endif
#include <nRF24L01.h>                     // Library used for the communication between the microcontroller and NRF24
#include <RF24.h>                         // Manin Library for interact with the NRF RADIO
#include "I2Cdev.h"                       // Support Library for I2C communication

RF24 radio(7, 8);                         // Creation for an object radio setting i pin CE=7 e CSN=8 used for the SPI connetion (you can chose every pin)
bool dmpReady = false;                    // Flag to verify if the MPU is correctely configured

const byte address[6] = "TXSTE";          // Unique adress used for communication between NRF24 radio and Arduino
struct DataPacket {                       // Structure that contains x=roll,y=pitch and z=yaw
  byte x;                                 // Value x axis (Roll) mapped 0-254
  byte y;                                 // Value y axis (Pitch) mappato 0-254
  byte z;                                 // Value z axis (Yaw) mapped 0-254
};
DataPacket data;                          // Creation of an instance of the structure called 'data'

uint8_t fifoBuffer[64];               // uint8_t intero a 8 bit: i have 64 cels 8 bit(1Byte)  each  // Buffer that contains data that comee from the processor (DMP) inside MPU
Quaternion q;                             // Variable for memorizinng rotation data  in quaternion format
VectorFloat gravity;                      // Variable pfor memorizing gravity value 
float ypr[3];                            // Array that contains final angles in degrees: [Yaw, Pitch, Roll]
MPU6050 mpu;                              // Creation of an object mpu in order to interact with the sensor

void setupMPU(){
  // Inizialization of I2C comunication
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE // if row 61 in I2Cdev.h I2CDEV_IMPLEMENTATION is set to I2CDEV_ARDUINO_WIRE enble the bus I2C
      Wire.begin();                       // Enables I2C bus as a master
      Wire.setClock(400000);              // Set the  velocity to 400kHz of I2C bus  (Fast Mode) in order to empty the buffer very fast avoiding data loss
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif
  Wire.setWireTimeout(3000, true); // EMI problems 
  mpu.initialize();                       // Inizialization of the MPU sensor
  int devStatus = mpu.dmpInitialize();    // Inizialization of the  Digital Motion Processor (DMP) 
  // If the inizialization is good  dev status is set to 0 by mpu.dmpInitialize()

  if (devStatus == 0)                    
  {
      mpu.CalibrateAccel(6);              // Calibration of  the aaccelerometer (6 cicle of calibration to get the perfect zero condiotion )
      mpu.CalibrateGyro(6);               // Calibration of gYROSCOPE (6 cicle of calibration to get the perfect zero condition)
      mpu.setDMPEnabled(true);            // Enable DMP process to acquire and process data
      dmpReady = true;                    // Set flag = TRUE: MPU is ready to acquire data
  }
}

void setup() {
  Serial.begin(9600);                     // Start the serial comunication for the debug at 9600 baud ()
  Serial.println("=== TRASMETTITORE ===");
  
  if (!radio.begin()) {                   // Inizialization of the radio module; se fallisce entra nel blocco
    Serial.println("Modulo NRF24 NON trovato!");
    while (1);                          
  }
  
  radio.setChannel(108);                  // Set the operating frequnency of the to channel 108 (fuori banda WiFi standard)
  radio.setDataRate(RF24_250KBPS);        // Velocity of  transmission low to increase stability and the distance TX-RX
  radio.setPALevel(RF24_PA_MIN);      
  radio.setRetries(5, 15);                // Try to send 15 times a packet [roll,pitch, yaw] for  5*250 microsecondi (250 us) per understand if  se the packet is sent to RX 
  /*The value of 250 μs is the minimal delay to:

  Sent a small packet

  Switch from Transmissione modality to the receiver modality

  Receive the ACK signal from the receiver

  NOte that TX nee to return to transmission modality if ACK is not arrived yet from RX to TX.
  // The packet need  to reach destination  250us minimum indeed if th value is lower than  250 
  (for instance  100 μs), the chip will not have time to switch from transmission modality to reciver modality and doesn't receive ACK even if the ACK is correctely sent from RX */
  
  radio.openWritingPipe(address);         // Open a comunication channel asociated to the adress i have already defined
  radio.stopListening();                  // Set the  modulo in TX modality 
  Serial.println("Setup completato!");
  setupMPU();                             // Chiama la funzione di configurazione del sensore di movimento
}

void loop() {
  if (!dmpReady) return;                  // If MPU is not ready exit from loop

  // Check if a packet is available in the buffer
  //fifobuffer=[Quatenrions:W,X,Y,Z,Gyro: ,X,Y,Z,Acdel:X,Y,Z]
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { 
    mpu.dmpGetQuaternion(&q, fifoBuffer);         // Extraction of  quaternioni from buffer
    mpu.dmpGetGravity(&gravity, &q);              // Calculation of  gravity vector based on quaternions
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);    // Conversion of data from Euleaian angle(Yaw/Pitch/Roll) to radians
    
    // Conversion from radians to  degrees all values must be between -50 and 50 if value <-50 the value is set to- 50 if value >50 the vlaue is set to 50
    int xAxisValue = constrain(ypr[2] * 180/M_PI, -50, 50); 
    int yAxisValue = constrain(ypr[1] * 180/M_PI, -50, 50);
    int zAxisValue = constrain(ypr[0] * 180/M_PI, -50, 50);    
    
    // Mapping (-50/+50) in a range 0-254 that occupy onlu 1 byte (small packet transmissio is faster )
    data.x = map(xAxisValue, -50, 50, 0, 254); 
    data.y = map(yAxisValue, -50, 50, 0, 254);
    data.z = map(zAxisValue, -50, 50, 0, 254);  
    mpu.resetFIFO();  // aggunto ora 
  }

  // Invia la struttura 'data' via radio e salva l'esito (true/false) in 'ok'
  bool ok = radio.write(&data, sizeof(data));

// Serial debug
  Serial.print("X = "); Serial.print(data.x);
  Serial.print(" | Y = "); Serial.print(data.y);
  Serial.print(" | Z = "); Serial.println(data.z);

  if(data.y<75){
    if (data.x < 75) {
      Serial.println(" GO FORWARD LEFT");
    } 
    else if (data.x > 175) {
      Serial.println(" GO FORWARD RIGHT");
    }
    else {
    
      Serial.println("GO FORWARD");
    }
     

  }

  else if (data.y > 175)  // indietro
  {
    if (data.x < 75) {

      Serial.println(" GO BACKWARDS LEFT");
    } 
    else if (data.x > 175) {
      Serial.println(" GO BACKWARDS RIGHT");
    } 
    else {
      Serial.println(" GO ONLY BACKWARDS");
    }
  }
  else if (data.z > 175)
  {
      //ruotaDestra(sx, dx);  extra implementation
      Serial.println("STOP");
  }
  else if (data.z < 75){
    //ruotaSinistra(sx, dx); extra implementation
        Serial.println("STOP");

  }
  else
  {
        Serial.println("STOP");
    
  }

  
  if(ok){
    Serial.println("DRIVING CONTROL SENT"); // data sent to RX correctely ACK received
  }
  else{
    Serial.println("COMMUNICATION ERROR");   // transmission error ACK not received yet
  }
}