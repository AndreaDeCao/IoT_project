// #include <IRremote.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>   //screen library

#include "velox.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Non usato su ESP32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//global variables (they are used in more functions) all function can access to them

//volatile --> per modificare negli interrupt, prende da SRAM e non da registro
volatile unsigned long tempo1;
volatile unsigned long tempo2;
volatile bool passaggio1=true; // ir barrier 1 on  (no car detected yet)
volatile bool passaggio2=true; // ir barrier 2 on (no car detected yet)

#define IR_SENSOR1 18
#define IR_SENSOR2 19


typedef enum{
  WAIT,
  IR1,
  IR2,
  COMPUTATION,
  NUM_STATES
}State_v;



typedef struct{ 
    State_v state;
    void (*state_function)(void); // chiama la funzione relativa allo stato
} StateMachine_t;



StateMachine_t fsm[] = { 
                      {WAIT, fn_START}, // se sono stato wait chiamo funzione start
                      {IR1, fn_BAR1}, //" " 
                      {IR2, fn_BAR2},
                      {COMPUTATION, fn_RESULT}
};


State_v current_state = WAIT;

// ISR: handle the interruot the car is detected at the first barrier
void ISR_SENSOR1() {
  passaggio1 = false; // macchina passata
}

// ISR: handle the interruot the car is detected at the first barrier
void ISR_SENSOR2() {
  passaggio2 = false;
}

// function where i start the sampling 
void fn_START() {
  Serial.println("start");
  YellowOn();      
  current_state = IR1; 
}
 // function that sample the instant where car pass through the first IR barrier
void fn_BAR1() {
  int x = 0;
  // use this two lines to test the hardware but you will use the value from sensor when we will have it
  //passaggio1=false; // car detected forzato
  //passaggio1=true //stay in the loop (car is not detected yet)
    while (/*digitalRead(IR_LEDS_REC[0]==HIGH)*/passaggio1) {
      display.clearDisplay();
      BlueOn();
      // irsend2.sendRaw(rawData, sizeRaw, CARRIER_FREQ);
      display.drawBitmap(x, 25, wifiSymbol, 32, 32, SSD1306_WHITE);
      display.display();
      x++;
      if(x > SCREEN_WIDTH - 32) x = 0;
      delay(100);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("auto rilevata  a barriera 1");
    display.display();  
    delay(1000);
    tempo1 = micros(); // questo ora è preciso
    current_state=IR2;
    
}
 // function that sample the instant where car pass through the second IR barrier
void fn_BAR2() {
   int pixel_growing = 0;
   // use this two lines to test the hardware but you will use the value from sensor when we will have it
   passaggio2=false; // car detected
   //passaggio2=true; stay in the loop (car is not detected yet)
    while (/*digitalRead(IR_LEDS_REC[1]==HIGH)*/passaggio2) {
      display.clearDisplay();
      GreenOn();
      //irsend2.sendRaw(rawData, sizeRaw, CARRIER_FREQ);
      display.drawBitmap(x, 25, wifiSymbol, 32, 32, SSD1306_WHITE);
      display.display();
      pixel_growing++;
      if(pixel_growing > SCREEN_WIDTH - 32) pixel_growing = 0; // condtion to stay within the lcd width dimension
      delay(100);
      
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("auto rilevata  a barriera 2");
    display.display();  
    delay(1000);
    tempo2 = micros(); // questo ora è preciso
    current_state = COMPUTATION; // Torna allo stato iniziale


}

// function tha compute the velocity of the car and handle the eventually amount of the bill in case of excess of velocity

void fn_RESULT() {
    noInterrupts(); // stop interrupts in order to collect samples of tempo1 and tempo 2 (without thi line tempo1 and tempo 2 can be overwrited and we measure a wrong sample)
    unsigned long t1 = tempo1;
    unsigned long t2 = tempo2;
    interrupts(); // enable interrupts to start new sampling
        

    // Calcolo tempo e velocità
    float t_secondi = (t2-t1) / 1000000.0; // da micros a secondi
    float velocita12 = distanza12_cm / t_secondi; // cm/s

    Serial.print("Tempo (s): ");
    Serial.println(t_secondi, 6);
    Serial.print("Velocita (cm/s): ");
    Serial.println(velocita12, 2);

    // Se velocità supera soglia, trasmetti con IR3
    velocita12=51;
    if (velocita12 > sogliaVelocita) {
        RedOn();   
        irsend3.sendRaw(rawData, sizeRaw, CARRIER_FREQ);
        Serial.println("IR3 inviato: velocita oltre soglia!");
    } else {
        digitalWrite(IR_LEDS_TRANS[2], LOW); // LED3 spento se velocità ok
        Serial.println("Velocita ok, LED3 spento.");
    }
    
    bool check_vbool=true;
  

    if ( /*digitalRead(IR_LEDS_REC[2])== HIGH*/check_vbool){
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("limite velocità superato !!");
      display.display();
      delay(1000);
      digitalWrite(LED_DEBUG[3],LOW);
    }
    // reset parameter for a new measuremnt
    tempo1 = 0;
    tempo2 = 0;
    passaggio1=true;
    passaggio2=true;
    current_state = WAIT; // go back to the first state: WAIT
}

void setup() {
  Serial.begin(9600);

    //Display initialization
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED non trovato!");
    // for(;;); // infinite loop 
    
    return ; // "error initialization SSD1306_SWITCHCAPVCC"
  }

  //uso pint 21,22 per comunicszione tra eesp e schermo tramite protocollo  I2C
  Wire.begin(21,22); // SDA, SCL for I2C communiction used in LCD



  // enable iNterrupt when infrared signal is RETURNED (logic level of the digital pin associated to IR sensor  goes HIGH->LOW) 
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR1 ), ISR_SENSOR1, FALLING); // FALLING TRANSIZIONE 1->0 LOGICO
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR2 ), ISR_SENSOR2, FALLING);
  
  


  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("sistema pronto!!");
  display.setTextSize(1);
  display.display();
}

void loop() {
    Serial.println("loop");
    // loop è l'equivalente del  "while(1)"
    if (current_state < NUM_STATES) {
        (*fsm[current_state].state_function)();

    } else {
        Serial.println("ERROR: invalid state!");
    }
}


void YellowOn(void){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("WAIT");
  display.display();  
  digitalWrite(LED_DEBUG[0],HIGH);
  delay(1000);
  digitalWrite(LED_DEBUG[0],LOW);
}

void BlueOn(void){
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("Sample 1"); 
  display.display();   
  digitalWrite(LED_DEBUG[1], HIGH);  
}

void GreenOn(void){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("Sample 2");
  display.display();  
  digitalWrite(LED_DEBUG[2],HIGH);

}

void RedOn(void){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("COMPIUTE");
  display.display();  
  digitalWrite(LED_DEBUG[3],HIGH);
  delay(1000);
}
