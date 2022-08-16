#include <StateMachine.h>
#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>
#include "HX711.h"
// define servo pins
#define srv1pin 9
#define srv2pin 6
#define srv3pin 3
#define srv4pin 5
// define ir pins
#define ir1pin 29
#define ir2pin 27
#define ir3pin 25
#define ir4pin 23
//define number of lanes 
#define laneNo 4
// define stepper motor controller pins
#define dirPin 11
#define stepPin 10
#define motorInterfaceType 1
#define calibration_factor4 759
#define calibration_factor3 762.40
#define calibration_factor2 773.50
#define calibration_factor1 771
#define HX711_dout_1 33
#define HX711_sck_1 35
#define HX711_dout_2 37
#define HX711_sck_2 39
#define HX711_dout_3 41
#define HX711_sck_3 43
#define HX711_dout_4 45
#define HX711_sck_4 47
#define powerPin 7
//#define sensorPin 8 // Temporary replace Webcam feedback
//pins:
#define onWebcamPin 53 //intend to change to 53
#define shredDonePin 52 //intend to change to 52
#define homeSwitch 13


//HX711 constructor (dout pin, sck pin)
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;




AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
Servo s1;
Servo s2;
Servo s3;
Servo s4;

const int STATE_DELAY = 1000;
// a list of 4 elements, each containing the status of the lane according to its index
// i.e. red lane is fullStatus[0]
int fullStatus[laneNo] = {0,0,0,0};
// a list of the irPins
int irPinArray[laneNo] = {29,27,25,23};
// a list of the stepper motor initial position. 
// i.e white is at pos 0
int stepperPos[laneNo] = {0,50,100,150}; // W,R,B,Y
int movingIndex;
bool isShredderOn = false;
unsigned long previousMillis = 0;
const long Interval = 1000;
int timer = 15;
// initial position for the servo
int pos = 0;
int shredderCycle = 0;
int colour;
int loadcell_current[4] = {0,0,0,0};
int loadcell_previous[4] = {0,0,0,0};
int setAllowance = 10;
String output[4] = {"Red","Blue","Yellow","White"};
long initial_homing = -1;
bool isHome = false;


// setup for state machine and its containing states.
// each state is a function with its associated state
// i.e. state 0 (S0) is a function called idleState
StateMachine machine = StateMachine();
State* S0 = machine.addState(&idleState);
State* S1 = machine.addState(&whiteLaneState);
State* S2 = machine.addState(&redLaneState);
State* S3 = machine.addState(&blueLaneState);
State* S4 = machine.addState(&yellowLaneState);
State* S5 = machine.addState(&shreddingState);
//State* S6 = machine.addState(&clearAll);e
State* S6 = machine.addState(&startWeighState);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
//  Wire.begin();
  pinMode(ir1pin, INPUT_PULLUP);
  pinMode(ir2pin, INPUT_PULLUP);
  pinMode(ir3pin, INPUT_PULLUP);
  pinMode(ir4pin, INPUT_PULLUP);
  pinMode(homeSwitch, INPUT_PULLUP);
  s1.attach(srv1pin);
  s2.attach(srv2pin);
  s3.attach(srv3pin);
  s4.attach(srv4pin);
  s1.write(pos);
  s2.write(pos);
  s3.write(pos);
  s4.write(pos);

  goHome();
  setSpeed_Acc();
  // set up transition between states
  S0->addTransition(&transitionS0S1,S1); //i.e. transtion from state 0 (idleState) to state 1 (whiteLaneState)
  S0->addTransition(&transitionS0S2,S2);
  S0->addTransition(&transitionS0S3,S3);
  S0->addTransition(&transitionS0S4,S4);
  S1->addTransition(&transitionS1S5,S5);
  S2->addTransition(&transitionS2S5,S5);
  S3->addTransition(&transitionS3S5,S5);
  S4->addTransition(&transitionS4S5,S5);
//  S5->addTransition(&transitionS5S0,S0);
  S5->addTransition(&transitionS5S6,S6);
  S5->addTransition(&transitionS5S5,S5);
  S5->addTransition(&transitionS5S0,S0);
//  S6->addTransition(&transitionS6S7,S7);
  S6->addTransition(&transitionS6S0,S0);

  scale1.begin(HX711_dout_1, HX711_sck_1);
  scale2.begin(HX711_dout_2, HX711_sck_2);
  scale3.begin(HX711_dout_3, HX711_sck_3);
  scale4.begin(HX711_dout_4, HX711_sck_4);

  scale1.set_scale(calibration_factor1); 
  scale1.tare();
  scale2.set_scale(calibration_factor2);
  scale2.tare();
  scale3.set_scale(calibration_factor3);
  scale3.tare();
  scale4.set_scale(calibration_factor4); 
  scale4.tare();

  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
//  pinMode(sensorPin, INPUT);

  pinMode(onWebcamPin, OUTPUT);
  digitalWrite(onWebcamPin, LOW);
  pinMode(shredDonePin,INPUT);

  Serial.println("Startup is complete");

  
}

void loop() {
  // put your main code here, to run repeatedly:
  machine.run();
  delay(STATE_DELAY);
  if (digitalRead(homeSwitch)){
    
  }
}

// the idle state is the initial looping state, which records the IR sensor input
void idleState(){
  Serial.println("idleState");
    for (int i=0; i<4; i++){
    Serial.println("This is ir sensor no. " + String(i));
    if (digitalRead(irPinArray[i])==LOW){
      Serial.println("check if lane no. "+ String(i) + " is full");
      int store = int(digitalRead(irPinArray[i]));
      delay(100); // might need to change to 3000 = 3seconds
      if (store == int(digitalRead(irPinArray[i]))){
        Serial.println("Lane no. "+ String(i) + " is full");
        fullStatus[i] = 1;
        Serial.println(String(fullStatus[i]));
      }else{
        fullStatus[i] = 0;
      }
    }else{
      fullStatus[i] = 0;
    }
 
  }
  // should I consider variety (movingIndex) or should I consider load? 
  // (i.e. most common colour is always checked first
}

// transitions are Instantiated based on which one is triggered first
// beginning with the most common colour

// each transition checks which of the lane fullStatus is of value 1
bool transitionS0S2(){
  if (fullStatus[0]){
    colour = 1;
    return true;
  }
  else{
    return false;
  }
}

bool transitionS0S3(){
  if (fullStatus[1]){
    colour = 2;
    return true;
  }
  else{
    return false;
  }
}

bool transitionS0S4(){
  if (fullStatus[2]){
    colour = 3;
    return true;
  }
  else{
    return false;
  }
}

bool transitionS0S1(){
  if (fullStatus[3]){
    colour = 4;
    return true;
  }
  else{
    return false;
  }
}

// this is the state which ensures the path from storage -> shredder -> flake collection are alligned correctly based on color, in this case white
void whiteLaneState(){
  Serial.println("whiteLaneState");
  // instantiates the moveStepper function to ensure stepper moves to the correct flake collection color
  moveStepper(stepperPos[3],3);
  // servo motor moves accordingly to release items in white lane
  for (pos = 0; pos <= 180; pos+=1){
    Serial.println("Servomotor 4 is at pos: " + String(pos));
    s4.write(pos);
    delay(15);
  }
  for (pos = 180; pos >= 0; pos -=1){
    Serial.println("Servomotor 4 is at pos: " + String(pos));
    s4.write(pos);
    delay(15);
  }
}

// transtion to the shredder state. Include code here to turn on the shredder
bool transitionS1S5(){
  // add code to turn on shredder here
  Serial.println("on the shredder");
  isShredderOn = true;
  digitalWrite(powerPin, LOW);
  digitalWrite(onWebcamPin, HIGH);
  return true;
}

void redLaneState(){
  Serial.println("redLaneState");
  moveStepper(stepperPos[0],0);
  for (pos = 0; pos <= 180; pos+=1){
    Serial.println("Servomotor 1 is at pos: " + String(pos));
    s1.write(pos);
    delay(15);
  }
  for (pos = 180; pos >= 0; pos -=1){
    Serial.println("Servomotor 1 is at pos: " + String(pos));
    s1.write(pos);
    delay(15);
  }
}

bool transitionS2S5(){
  // add code to turn on shredder here
  Serial.println("Shredder is on");
  isShredderOn = true;
  digitalWrite(powerPin, LOW);
  digitalWrite(onWebcamPin, HIGH);
  return true;
}


void blueLaneState(){
  Serial.println("blueLaneState");
  moveStepper(stepperPos[1],1);
  for (pos = 0; pos <= 180; pos+=1){
    Serial.println("Servomotor 2 is at pos: " + String(pos));
    s2.write(pos);
    delay(15);
  }
  for (pos = 180; pos >= 0; pos -=1){
    Serial.println("Servomotor 2 is at pos: " + String(pos));
    s2.write(pos);
    delay(15);
  }
}

bool transitionS3S5(){
  // add code to turn on shredder here
  Serial.println("Shredder is on");
  isShredderOn = true;
  digitalWrite(powerPin, LOW);
  digitalWrite(onWebcamPin, HIGH);
  return true;
}

void yellowLaneState(){
  Serial.println("yellowLaneState");
  moveStepper(stepperPos[2],2);
  for (pos = 0; pos <= 180; pos+=1){
    Serial.println("Servomotor 3 is at pos: " + String(pos));
    s3.write(pos);
    delay(15);
  }
  for (pos = 180; pos >= 0; pos -=1){
    Serial.println("Servomotor 3 is at pos: " + String(pos));
    s3.write(pos);
    delay(15);
  }
}

bool transitionS4S5(){
  // add code to turn on shredder here
  Serial.println("Shredder is on");
  isShredderOn = true;
  digitalWrite(powerPin, LOW);
  digitalWrite(onWebcamPin, HIGH);
  return true;
}

// this is a countdown timer to ensure shredder is on for specified time
// each time it loops back, countdown timer duration should decrease
void shreddingState(){
  Serial.println("shreddingState");
  Serial.println("Current cycle is: " + String(shredderCycle));
  Serial.println("Current timer is: " + String(timer));
  // shredder should be on upon reaching this state. Now to monitor
  // and check if shredding has finished
  // for now, I replace this with a shredder_sim (aka timer)
  while (isShredderOn){
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= Interval){
      previousMillis = currentMillis;
      Serial.println(timer);
      Serial.println("Shredder is ongoing");
      timer--;
    }
    if (timer == 0){
    Serial.println("Shredder has ended");
    if (shredderCycle == 0){
      Serial.println("1st Shredder Cycle is done");
      timer = 10;
    }else if (shredderCycle == 1){
      Serial.println("2nd Shredder Cycle is done");
      timer = 5;
    }else if (shredderCycle == 2){
      Serial.println("3rd Shredder Cycle is done");
      timer = 2;
    }else if (shredderCycle == 3){
      Serial.println("Shredder ran 4x, smth is wrong");
    }
    break;
    }
  }
}

//bool transitionS5S0(){
//  // add code to turn off shredder here
//  Serial.println("Shredder is off");
//  isShredderOn = false;
//  return true;
//}

// check the webcam feedback, if no more waste to shred, move to next state
bool transitionS5S6(){
// if webcam feedback is LOW
// reset the cycle to zero
// reset the timer to original
// move on to next state
  if (digitalRead(shredDonePin) == LOW){
    Serial.println("No more things to shred, can turn off shredder");
    digitalWrite(powerPin, HIGH);
    digitalWrite(onWebcamPin, LOW);
    isShredderOn = false;
    timer = 15;
    shredderCycle = 0;
    return true;
  }else{
    return false;
  }
}

bool transitionS5S0(){
  if (shredderCycle >= 3){
    Serial.println("Catch Shredder error here!");
    digitalWrite(powerPin, HIGH);
    digitalWrite(onWebcamPin, LOW);
    isShredderOn = false;
    timer = 15;
    shredderCycle = 0;
    return true;
  }else{
    return false;
  }
}

// check the webcam feedback, if still have waste to shred, loop back to S5
bool transitionS5S5(){
// if webcam feedback is HIGH
// increase the cycle
// return back to previous state
  if (digitalRead(shredDonePin) == HIGH && shredderCycle < 3){
    Serial.println("Still have remaining things to shred, returning to shredder state");
    shredderCycle++;
    return true;
  }else{
    return false;
  }
}

// this state is to clear the shredder, or run any logic
// that is required to reset the shredder and prep it
// for being used again. If don't have, skip. 
//void clearAll(){
//
//}

// this state is to take measurement of the weight
// obtain the input weight and send it back to the master
void startWeighState(){
  Serial.println("startWeighState");
  Serial.println("Colour is: "+String(colour));
  int index_Colour;
  float collectedWeight;
  loadcell_current[0] = round(scale1.get_units(5));
  loadcell_current[1] = round(scale2.get_units(5));
  loadcell_current[2] = round(scale3.get_units(5));
  loadcell_current[3] = round(scale4.get_units(5));
  Serial.print("Load_cell 1 output val: ");
  Serial.print(loadcell_current[0]);
  Serial.print("      Load_cell 2 output val: ");
  Serial.print(loadcell_current[1]);
  Serial.print("      Load_cell 3 output val: ");
  Serial.print(loadcell_current[2]);
  Serial.print("      Load_cell 4 output val: ");
  Serial.println(loadcell_current[3]);
  for (int i = 0; i <4; i++){
    if ((loadcell_current[i] - loadcell_previous[i]) > setAllowance){
      collectedWeight = loadcell_current[i] - loadcell_previous[i];
      if (colour == 1){
        index_Colour = 0;
      }else if (colour == 2){
        index_Colour = 1;
      }else if (colour == 3){
        index_Colour = 2;
      }else if (colour == 4){
        index_Colour = 3;
      }
      // TO BE SENT TO RPI
      Serial.println("The collected weight for loadcell " + String(i) + " is " + String(collectedWeight));
      loadcell_previous[i] = loadcell_current[i];
    }
  }
  // to read to print out loadcell_previous array
  for (int i = 0; i <4; i++){
    Serial.println("loadcell_previous index " + String(i) + " is " + String(loadcell_previous[i]));
  }
  if (collectedWeight > 0){
    Serial.println("Hello, StrShrCol,"+String(collectedWeight)+","+output[index_Colour]);
  }else{
    Serial.println("Hello, StrShrCol,0,ERROR");
  }
  
}

bool transitionS6S0(){
  return true;
}



// this is the function that moves the stepper accordingly.
// recieves 2 args: stepperPos[i] where i is which pos each color is currently at 
// and arrayIndex as an int that indicates which coloured lane is releasing (i.e. 0 is white lane)
void moveStepper(int stepperPosIn, int arrayIndex){
  Serial.println("moveStepper has been called to pos " + String(stepperPosIn));
  const int ogStepperPos[4] = {0,50,100,150};
  if (stepperPosIn == 50 || stepperPosIn == 100){ //turn CW to position respectively
    stepper.moveTo(stepperPosIn);
    stepper.setSpeed(100);
    stepper.runToPosition();
  }else if (stepperPosIn == 150){ //turn CCW to position 
    stepper.moveTo(-60);
    stepper.setSpeed(-100);
    stepper.runToPosition();
  }
  delay(1000);
  if (stepper.distanceToGo() == 0){
    stepper.setCurrentPosition(0);
    // once stepper has moved to a new position, the pos of each color is reset
    // with the current color chosen taking pos 0, the pos is shifted accordingly 
    for (int i = 0; i <= 3; i++){
      if (arrayIndex > 3){
        arrayIndex = arrayIndex - 4;
      }
      stepperPos[arrayIndex] = ogStepperPos[i];
      Serial.println("At " + String(arrayIndex) + " pos has been changed to: " + String(ogStepperPos[i]));
      arrayIndex++;
    }
  }
}


// for taring all 4 load cell without delay
//void tare_4x(){
//  if(Serial.available() > 0){
//    char inByte = Serial.read();
//    if(inByte == 't'){
//      LoadCell_1.tareNoDelay();
//      LoadCell_2.tareNoDelay();
//      LoadCell_3.tareNoDelay();
//      LoadCell_4.tareNoDelay();
//    }
//  }
//  if(LoadCell_1.getTareStatus() == true){
//    Serial.println("Tare load cell 1 complete");
//  }
//  if(LoadCell_2.getTareStatus() == true){
//    Serial.println("Tare load cell 2 complete");
//  }
//  if(LoadCell_3.getTareStatus() == true){
//    Serial.println("Tare load cell 3 complete");
//  }
//  if(LoadCell_4.getTareStatus() == true){
//    Serial.println("Tare load cell 4 complete");
//  }
//}

void goHome(){
  stepper.setMaxSpeed(50.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(50.0);  // Set Acceleration of Stepper
  Serial.print("Stepper is Homing . . . . . . . . . . . ");
  Serial.println(digitalRead(homeSwitch));
  while (digitalRead(homeSwitch)) {  // Make the Stepper move CCW until the switch is activated
    stepper.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepper.run();  // Start moving the stepper
    delay(5);
  }

  stepper.setCurrentPosition(0);  // Set the current position as zero for now
  stepper.setMaxSpeed(50.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(50.0);  // Set Acceleration of Stepper
  initial_homing = 1;

  while (!digitalRead(homeSwitch)) { // Make the Stepper move CW until the switch is deactivated
    stepper.moveTo(initial_homing);  
    stepper.run();
    initial_homing++;
    delay(5);
  }
  stepper.setCurrentPosition(0);
  Serial.println("Homing Completed");
  Serial.println("");
  isHome = true;
}

void setSpeed_Acc(){
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
}
