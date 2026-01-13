const int M1_PWM = 9;  // Direzione oraria motori 1 e 2 gestione da pin digitali (no pwm)
const int M2_PWM = 10;  // Direzione antioraria motori 3 e 4 da pin digitali (no pwm)
const int DIRA =2; // direzione motori 1 e 2
const int DIRB=4; // direzioen mptori 1 e 2 


void setup() {
  Serial.begin(9600);
  pinMode(M1_PWM, OUTPUT);
  pinMode(M2_PWM, OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
  

}

void loop() {
  digitalWrite(DIRA,HIGH);
  digitalWrite(DIRB,LOW);
  Serial.println("Motore 1 e 2 acceso");
  analogWrite(M1_PWM,255); // motore 1 gira in verso orario
  analogWrite(M2_PWM,255); // motore 2 gira in verso orario
  delay(2000);
  Serial.println("sinistra");
  analogWrite(M1_PWM,255); // motore 1 gira in verso orario
  analogWrite(M2_PWM,0); // motore 2 gira in verso orario
  delay(5000);
  Serial.println("destra");
  analogWrite(M1_PWM,0); // motore 1 gira in verso orario
  analogWrite(M2_PWM,255); // motore 2 gira in verso orario
  delay(5000);
  Serial.println("stop");
  analogWrite(M1_PWM,0); // motore 1 gira in verso orario
  analogWrite(M2_PWM,0); // motore 2 gira in verso orario
  delay(1000);

}

