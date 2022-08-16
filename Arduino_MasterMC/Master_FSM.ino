#include <StateMachine.h>
#include <Wire.h>
#include <FastLED.h>

#define SLAVE_SENSORPLT 8
#define SLAVE_SORTING 9
#define ir1pin 8
#define ir2pin 9
#define ir3pin 10
#define ir4pin 11
#define laneNo 4
#define LED_PIN1 5
#define LED_PIN2 6
#define NUM_LEDS1 8
#define NUM_LEDS2 2
#define BRIGHTNESS 64
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
CRGB fullLeds[NUM_LEDS1];
CRGB statusLeds[NUM_LEDS2];


const int STATE_DELAY = 1000;
//int randomState = 0;
//const int LED = 13;
float weight;
byte byte1, byte2;
int colour = 0;
int colourMap;
int received;
int requestColour;
int machineCount = 0;
int fullStatus[laneNo] = {0,0,0,0};
int irPinArray[laneNo] = {8,9,10,11}; //R,B,Y,W
//bool processing = false;

StateMachine machine = StateMachine();
//State* S0 = machine.addState(&start_calibrate);
State* S0 = machine.addState(&idleState);
State* S1 = machine.addState(&colourSensorState);
State* S2 = machine.addState(&awaitingObjectState);
State* S3 = machine.addState(&homingState);
State* S4 = machine.addState(&sortingState);

//State* S3 = machine.addState(&state3);
//State* S4 = machine.addState(&state4);

void setup() {
  Serial.begin(9600);
//  pinMode(LED,OUTPUT);
//  randomSeed(A0);
  Wire.begin();
  pinMode(ir1pin, INPUT_PULLUP);
  pinMode(ir2pin, INPUT_PULLUP);
  pinMode(ir3pin, INPUT_PULLUP);
  pinMode(ir4pin, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE, LED_PIN1, COLOR_ORDER>(fullLeds, NUM_LEDS1);
  FastLED.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(statusLeds, NUM_LEDS2);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  /*
  * The other way to define transitions.
  * (Looks cleaner)
  * Functions must be defined in the sketch
  */
  S0->addTransition(&transitionS0S0,S0);
  S0->addTransition(&transitionS0S1,S1);
  S1->addTransition(&transitionS1S0,S0);
  S1->addTransition(&transitionS1S2,S2);
  S2->addTransition(&transitionS2S2,S2);
  S2->addTransition(&transitionS2S3,S3);
  S2->addTransition(&transitionS2S0,S0);
  S3->addTransition(&transitionS3S4,S4);
  S4->addTransition(&transitionS4S0,S0);
//  S4->addTransition(&transitionS4S5,S5);
//  S5->addTransition(&transitionS5S0,S0);
//  S5->addTransition(&transitionS5S2,S2);
  //.println("Master start");
}

void loop() {
  isItFull();
  for (int i=0; i < 4; i++){
    if(fullStatus[i] == 0){
      switch (i){
        case 0:
          fullLeds[0] = CRGB::Red;
          fullLeds[1] = CRGB::Red;
          FastLED.show();
          //.println("Red is empty");
          break;
        case 1:
          fullLeds[2] = CRGB::Blue;
          fullLeds[3] = CRGB::Blue;
          FastLED.show();
          //.println("Blue is empty");
          break;
        case 2:
          fullLeds[4] = CRGB::Yellow;
          fullLeds[5] = CRGB::Yellow;
          FastLED.show();
          //.println("Yellow is emtpy");
          break;
        case 3:
          fullLeds[6] = CRGB::White;
          fullLeds[7] = CRGB::White;
          FastLED.show();
          //.println("White is empty");
          break;
      }
    }else if(fullStatus[i] == 1){
      switch (i){
        case 0:
          fullLeds[0] = CRGB::Black;
          fullLeds[1] = CRGB::Black;
          FastLED.show();
          //.println("Red is full");
          break;
        case 1:
          fullLeds[2] = CRGB::Black;
          fullLeds[3] = CRGB::Black;
          FastLED.show();
          //.println("Blue is full");
          break;
        case 2:
          fullLeds[4] = CRGB::Black;
          fullLeds[5] = CRGB::Black;
          FastLED.show();
          //.println("Yellow is full");
          break;
        case 3:
          fullLeds[6] = CRGB::Black;
          fullLeds[7] = CRGB::Black;
          FastLED.show();
          //.println("White is full");
          break;
      }
    }
  }
  machine.run();
  delay(STATE_DELAY);
}


// Do I need to start a calibration sequence before machine start? 
//void start_calibrate(){
//  
//}


void idleState(){
  //.println("idleState");
  fill_solid(statusLeds, NUM_LEDS2, CRGB::Yellow);
  FastLED.show();
  Wire.beginTransmission(SLAVE_SENSORPLT);
  Wire.write(0);
  Wire.endTransmission();
  delay(70);
  Wire.requestFrom(SLAVE_SENSORPLT,4);
  int i = 0;
  union weight_tag {char weight_b[4]; float weight_fval;} weight_Union;
  while (Wire.available()){ //if two bytes were recuieved
    weight_Union.weight_b[i] = Wire.read();
    i++;
  }
  weight = weight_Union.weight_fval;
  //.print("Reading: ");
  //.print(weight);
  //.print(" g");
  //.println();
}


bool transitionS0S0(){
  if (weight <= 0){
    //.println("Go back and weigh"); 
    return true;
  }
  else{
    return false;
  }
}

bool transitionS0S1(){
  if (weight > 0.1){ 
    return true;
  }
  else{
    return false;
  }
}

//-------------------------

void colourSensorState(){
  //.println("colour Sensor State");
  // transmit to slave 8 to perform colour sensing. 2nd prototype will involve material sensing
  Wire.beginTransmission(SLAVE_SENSORPLT);
  Wire.write(1);
  Wire.endTransmission();
  delay(1000);
  Wire.requestFrom(SLAVE_SENSORPLT,1);
//  checkSlaveres();
//  //.println(sizeof(Wire.available()));
  while (Wire.available()){
    colour = int(Wire.read());
//    //.println(colour);
    if (colour == 1) {
    //.println("Red");
    colourMap = 11;
    }
    else if (colour == 2) {
    //.println("Blue");
    colourMap = 12;
    }
    else if (colour == 3) {
    //.println("Yellow");
    colourMap = 13;
    }
    else if (colour == 4) {
    //.println("White");
    colourMap = 14;
    }
    else if (colour >= 5 || colour == 0){
    //.println("Unknown colour");
    colourMap = 15;
    }
  }
}

bool transitionS1S0(){
  if (colour >= 5 || colour == 0){
    //.println("Got rejected, unidentified colour");
    redBlinking();
    return true;
  }else{
//    isItFull();
    for (int i=0; i<4; i++){
      //.print(fullStatus[i]);
    }
    if (fullStatus[0] == 1 && colour == 1){
      //.println("Got rejected because red lane colour is full");
      redBlinking();
      return true;
    }else if (fullStatus[1] == 1 && colour == 2){
      //.println("Got rejected because blue lane colour is full");
      redBlinking();
      return true;
    }else if (fullStatus[2] == 1 && colour == 3){
      //.println("Got rejected because yellow lane colour is full");
      redBlinking();
      return true;
    }else if (fullStatus[3] == 1 && colour == 4){
      //.println("Got rejected because white lane colour is full");
      redBlinking();
      return true;
    }else{
      return false;
    }
  }
}

bool transitionS1S2(){
  // item is accepted by sensor platform, transition to sorting state
  char output;
  if (colour < 5 && colour > 0){
    if (colour == 1){
      char output = "Red";
    }else if (colour == 2){
      char output = "Blue";
    }else if (colour == 3){
      char output = "Yellow";
    }else if (colour == 4){
      char output = "White";
    }
    Serial.println("Hello, MasterArd,"+String(weight)+","+output);
    delay(700);
    Wire.beginTransmission(SLAVE_SENSORPLT);
    Wire.write(2);
    Wire.endTransmission();
    delay(70);
    fill_solid(statusLeds, NUM_LEDS2, CRGB::Green);
    FastLED.show();
    return true;
  }else{
    return false;
  }
}

void awaitingObjectState (){
  //.println("awaitingObjectState");
  colour = 0;
  Wire.beginTransmission(SLAVE_SORTING);
  Wire.write(3);
  Wire.endTransmission();
  delay(70);
  Wire.requestFrom(SLAVE_SORTING,1);
  while (Wire.available()){
    received = int(Wire.read());
    //.println("From sorting: " + String(received));
  }
//  delay(1000);
}
//
bool transitionS2S2(){
  //.println(received);
  if (received == 0){
    //.println("looping back to await Object");
    return true;
  }else{
    return false;
  }
}

bool transitionS2S3(){
  if (received == 1){
    //.println("Sorting System received the object");
    Wire.beginTransmission(SLAVE_SENSORPLT);
    Wire.write(3);
    Wire.endTransmission();
    delay(70);
    fill_solid(statusLeds, NUM_LEDS2, CRGB::OrangeRed);
    FastLED.show();
    return true;
  }else{
    return false;
  }
}

bool transitionS2S0(){
  if (received == 2){
    //.println("Never recieve object, time up");
    Wire.beginTransmission(SLAVE_SENSORPLT);
    Wire.write(3);
    Wire.endTransmission();
    delay(70);
    return true;
  }else{
    return false;
  }
}
//
////-------------------------
void homingState(){
  //.println("homingState");
  Wire.beginTransmission(SLAVE_SORTING);
  Wire.write(4); 
  Wire.endTransmission();
  delay(70);
  Wire.requestFrom(SLAVE_SORTING,1);
  while (Wire.available()){
    received = int(Wire.read());
    //.println("From sorting: " + String(received));
  }
//  if (received == 3){
//    //.println("Recieved 3");
//    Wire.beginTransmission(SLAVE_SORTING);
//    Wire.write(byte(colourMap));
//    Wire.endTransmission();
//    delay(70);
//    Wire.requestFrom(SLAVE_SORTING,1);
  }
  
//
bool transitionS3S4(){
  if (received == 3){
//    //.println("Home already, time to sort!");
    //.println(colourMap);
    Wire.beginTransmission(SLAVE_SORTING);
    Wire.write(byte(colourMap));
    Wire.endTransmission();
    delay(70);
    return true;
  }else{
    return false;
  }
}
//
////-------------------------
void sortingState(){
  //.println("sortingState");
  Wire.requestFrom(SLAVE_SORTING, 1);
  delay(70);
  while (Wire.available()){
    received = int(Wire.read());
    //.println("From sorting: " + String(received));
  }
}
//
bool transitionS4S0(){
  if (received == 4){
    machineCount += 1;
    //.println("Number of rotations: " + String(machineCount));
    fill_solid(statusLeds, NUM_LEDS2, CRGB::Black);
    FastLED.show();
    return true;
  }else if (received == 5){
    //.println("Still sorting...");
  }
  return false;
}
//
//bool transitionS5S2(){
//  return true;
//}

void checkSlaveres() {
  while (Wire.available()) {
    int x = Wire.read();
    //.println(x);
  }
}

void isItFull() {
  for (int i=0; i<4; i++){
    //.println("This is ir sensor no. " + String(i));
    if (digitalRead(irPinArray[i])==LOW){
      //.println("check if lane no. "+ String(i) + " is full");
      int store = int(digitalRead(irPinArray[i]));
      delay(100);
      if (store == int(digitalRead(irPinArray[i]))){
        //.println("Lane no. "+ String(i) + " is full");
        fullStatus[i] = 1;
        //.println(String(fullStatus[i]));
      }else{
        fullStatus[i] = 0;
      }
    }else{
      fullStatus[i] = 0;
    }
  }
}

void redBlinking(){
  for (int i = 0; i < 5; i++){
    fill_solid(statusLeds, NUM_LEDS2, CRGB::Red); 
    FastLED.show();
    delay(500);
    fill_solid(statusLeds, NUM_LEDS2, CRGB::Black);
    FastLED.show();
    delay(500);
  }
}

//void orangeBlinking(){
//  //.println("orange blinking is called");
//  //.println(processing);
//  if (processing){
//    unsigned long tymNow = millis();
//    int num2nds = (tymNow/1000);
//    if(num2nds % 2 == 0)fill_solid(statusLeds, NUM_LEDS2, CRGB::Orange);
//    if(num2nds % 2 > 0)fill_solid(statusLeds, NUM_LEDS2, CRGB::Black);
//    FastLED.show();
//  }
//  fill_solid(statusLeds, NUM_LEDS2, CRGB::Black);
//  FastLED.show();
//}
