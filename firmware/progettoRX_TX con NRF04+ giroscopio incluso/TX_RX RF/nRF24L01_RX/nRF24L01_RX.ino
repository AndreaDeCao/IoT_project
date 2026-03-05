#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN
const byte address[6] = "00001";
float angleX;
float angleY;
int speed;

// Pin motori
// Motori destra
#define ENA 5    // PWM che gestisce velocità motore A
#define IN1 2    // gestisce direzione motore A
#define IN2 4    // gestisce direzione motore A

// Motori di sinistra
#define ENB 6    // PWM che gestisce velocità motore B
#define IN3 3    // gestisce direzione motore B
#define IN4 9    // gestisce direzione motore B

struct DataPacket {
  float x;
  float y;
  float z;
};

DataPacket data;

void setup() {
  pinMode(ENA, OUTPUT); 
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.begin(9600);

  Serial.println("Avvio ricevitore");
  if (!radio.begin()) {
    Serial.println("ERRORE: Hardware Radio non trovato!");
    while (1); 
  }


  Serial.println("Ricevitore pronto!");
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
    radio.read(&data, sizeof(data));
    // Debug dati ricevuti
    Serial.print("RX X: "); Serial.print(data.x);
    Serial.print(" | Y: "); Serial.print(data.y);

    // Mostra anche le velocità calcolate
    int speed = map(abs(data.x), 0, 200, 0, 255);
    speed = constrain(speed, 0, 255);
    int steer = map(data.y, -200, 200, -100, 100);
    int leftSpeed = constrain(speed + steer, 0, 255);
    int rightSpeed = constrain(speed - steer, 0, 255);
    Serial.print("LeftSpeed: "); Serial.print(leftSpeed);
    Serial.print(" | RightSpeed: "); Serial.println(rightSpeed);
    movimento(data.x, data.y);
  }
}
void avanti();
void indietro();
void destra();
void sinistra();
void stopMotors();
// Movimento solo con X/Y
void movimento(int x, int y) {
  float angleX=data.x-0.57;
  float angleY=data.y-2.04;
  //const int DEAD = 5000; // zona di calibrazionne tr adestra e sinsitra
  int speed = map(abs(data.x), 0, 180, 0, 255);
  speed = constrain(speed, 0, 255);

  int steer = map(data.y, -200, 2000, -100, 100);
  int leftSpeed = constrain(speed + steer, 0, 255);
  int rightSpeed = constrain(speed - steer, 0, 255);

  // stop se X e Y vicino a zero
  /*if (abs(x) < DEAD && abs(y) < DEAD) {
    ferma();
    return;
  }

  // avanti/indietro con differenziale
  if (x > DEAD) {
    avanti(leftSpeed, rightSpeed);
  } else if (x < -DEAD) {
    indietro(leftSpeed, rightSpeed);
  }
}*/
if (angleX < 2 && angleX > -2 && angleY > -20 && angleY < 20) {
    Serial.println("STOP");
    stopMotors();
} 
else if (angleX < 0 && angleX<-3) {
    indietro();
    Serial.println("Indietro");
}
else if (angleX > 0 && angleY>1) {
    avanti();
    Serial.println("Avanti");
}
else if ((angleX>19)&& (angleY)>-3) {
    destra();
    Serial.println("Destra");
}
else if ((angleX<-70)&& (angleY)>0.00) {
    sinistra();
    Serial.println("Sinistra");
}
}

void avanti() {
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void indietro() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void destra() {
  // sinistro avanti, destro fermo
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, speed);   // motore sinistro
  analogWrite(ENB, 0);     // motore destro fermo
}

void sinistra() {
  // destro avanti, sinistro fermo
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);     // motore sinistro fermo
  analogWrite(ENB, speed);   // motore destro
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
/*// Funzioni motori con velocità differenziale
void avanti(int sx, int dx) {
  Serial.println("-> AVANTI");
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, sx);
  analogWrite(ENB, dx);
}

void indietro(int sx, int dx) {
  Serial.println("-> INDIETRO");
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, sx);
  analogWrite(ENB, dx);
}

void ferma() {
  Serial.println("-> STOP");
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
*/
