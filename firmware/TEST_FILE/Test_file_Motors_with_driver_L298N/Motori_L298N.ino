// ===== PIN =====

//right motors
#define ENA 5    // PWM tht handles the speed of right motors
// pin tha define the direction of right tires: clocwise or counter-clockwise
#define IN1 9 
#define IN2 10

// left motors
#define ENB 6    // PWM tht handles the speed of left motors
// pin tha define the direction of left tires: clocwise or counter-clockwise
#define IN3 2 
#define IN4 4 

// go forward/ go backward fixed value of speed = 100
int vel=100;

void setup() {
  // motor pins configuration
  pinMode(ENA, OUTPUT); 
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  avanti();
  delay(2000);

  stopMotors();
  delay(500);

  indietro();
  delay(2000);

  stopMotors();
  delay(500);
}

// ===== TEST FUNCTIONS =====

void avanti() {
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void indietro() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void destra() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);   // right  motors
  analogWrite(ENB, vel);     // left motors
}

void sinistra() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, vel);     // right motors
  analogWrite(ENB, 0);   // left motors
}


void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
