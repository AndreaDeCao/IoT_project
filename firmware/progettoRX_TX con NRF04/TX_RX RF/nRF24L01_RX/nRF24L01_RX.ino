#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN pin configuration NRF antenna
const byte address[6] = "TXSTE"; // RF library needs an adress of 5 characters or number (1Byte =charachter or number)
//Motor pins


// right motors A
#define ENA 5    // PWM pin that handles the speed of motor A
#define IN1 9    // digital pin that handle the direction of motor A
#define IN2 10    // digital pin that handle the direction of motor A

// left motors B
#define ENB 6    // PWM pin that handles the speed of motor B
#define IN3 2    // digital pin that handle the direction of motor B
#define IN4 4    // digital pin that handle the direction of motor B

// data structure that contain 3 variable
struct DataPacket {
  byte x; // x contains roll value (inclination on right /left side)
  byte y; // y contiene pitch value (back and forward)
  byte z; // z contains the yaw value(rotation // to xy plane)
};

DataPacket data;

// function declaration
void goForward(int sx, int dx);
void stop();
void goBackward(int sx, int dx);


void setup() {
  // setup of motor's pins 

  // motor A
  pinMode(ENA, OUTPUT); 
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);
  
  // motor B
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.begin(9600);
  // starting configuration of the receiver
  Serial.println("Avvio ricevitore");
  if (!radio.begin()) { 
    // radio.begin() => Arduino do three actions: 
    //enable SPI connections so it's set on the MISO, MOSI , SCK, CE, CSN pins
    // sent an electrical signal to the radio in order to bring it out from the power saving mode 
    // Arduino write a character in a small area of radio's memory using MOSI (Master Out, Slave In) , then make a request to the memory with the MISO (Master In, Slave Out), if the meory answer with the character wrote at the start it returns true otherwise false
    Serial.println("ERRORE: Hardware Radio non trovato!");
    while (1);  // infinite loop the radio if it doesn't respond
  }
  // if we pass this condition the receiver is ready 
  Serial.println("Ricevitore pronto!"); 
  radio.setChannel(108); // set the operating frequency where the transmitter and receiver can communicate without inteference. NRF radio work at 2.4GHz and setCHhannel set the operating frequency at 2.4Ghz+108Hz. 2.4Ghz is a crowded frequency so setChannel moves the operating frequency to an undisturbed frequency 
  radio.setDataRate(RF24_250KBPS); // 250KBPS = 250000 bits/s i can work at 1MBS  fast (RF24_1MBS =100000 milions bits /s) or 2MBS very fast (F24_2MBS =100000 milions bits /s)
  radio.setPALevel(RF24_PA_LOW); // set the power at which the ack is sent to tx to say ok i received x,y,z
  radio.openReadingPipe(0, address); // the NRF24 can listen on 6 channels at the same time: i set the 0 channel as the channel for the communications with the tx 
  radio.startListening(); // LNA (Low noise amplifier) is set on and PA Power amplifier is disable due to is used onlt in transmission of ack).
  // When i send ACK i disable the LNA and enable PA and after i sent ACK i disabled PA and enable LNA.
  Serial.println("Ricevitore pronto!");


}

void loop() {
  if (radio.available()) { // if a triplet of datas is available on the buffer radio.available() return TRUE
    radio.read(&data, sizeof(data)); // read data from the buffer
    //verify status of the 
    Serial.print("RX X: "); Serial.print(data.x);
    Serial.print(" | Y: "); Serial.print(data.y);
     Serial.print(" | Z: "); Serial.print(data.z);
    movimento(data.x, data.y);
  }
}

//
void movimento(int x, int y) {
  int moveX = map(x, 0, 254, -255, 255); // control the tires' direction and we map the roll values between the minimum and the maximum speed of the car 
  int moveY = map(y, 0, 254, -255, 255); // control the inclination of the mpu back/forward and we map the pitch's value between the minimum and the maximum speed of the car 
  int sx = constrain(abs(moveY + moveX), 0, 255); // differential velocity on left side (motors B) of the car 
  int dx = constrain(abs(moveY - moveX), 0, 255); // differential velocity on left side (motors B) of the car 

 // forward condition
if(data.y<75){
    if (data.x < 75) {
       goForward(sx, dx);
       Serial.println(" GO FORWARD LEFT");
      
    } 
    else if (data.x > 175) {
      goForward(sx, dx);
       Serial.println(" GO FORWARD RIGHT");
    }
    else {
      moveX=0;
      Serial.println("GO FORWARD");
      goForward(sx, dx);
    }
    
  }
  /////
  else if (data.y > 175)  // back condition
  {
    if (data.x < 75) {
      goBackward( sx, dx);
      Serial.println("GO BACKWARD LEFT");
    } 
    else if (data.x > 175) {
      goBackward(sx, dx);
      Serial.println(" GO BACKWARD RIGHT");
    } 
    else {
      goBackward(sx, dx);
      moveX=0;
      Serial.println(" GO ONLY BACKWARD");
    }
  }
  // rotation on z axis 
  else if (data.z > 175)
  {
      //ruotaDestra(sx, dx);
      stop();
      Serial.println("STOP");
  }
  else if (data.z < 75)
  {
    //ruotaSinistra(sx, dx);
    stop();
    Serial.println("STOP");

  }
  else
  {
    //processCarMovement(STOP);  
    stop();
    Serial.println("STOP");
    
  }
}
  


void goForward(int sx, int dx) {
  Serial.println("-> GO FORWARD");
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, sx);
  analogWrite(ENB, dx);
}

void goBackward(int sx, int dx) {
  Serial.println("-> GO BACKWARD");
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, dx);
  analogWrite(ENB, sx);
}

void stop() {
  Serial.println("-> STOP");
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}




