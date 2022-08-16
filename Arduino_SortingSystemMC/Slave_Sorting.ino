#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>

#define SLAVE_ADDR 9
#define IRSensor 4
#define dirPin 2
#define stepPin 5
#define motorInterfaceType 1
#define enPin 8
#define homeSwitch 13
//#define endSwitch 7
#define servoPin 3
#define posR 1281
#define posB 2416
#define posY 3562
#define posW 4696

int Master_command;
unsigned long previousMillis = 0;
const long Interval = 1000;
int timer = 30;
bool isHome = false;
int pos = 180;
bool sortingDone = false;

// stopwatch 
bool startState = false;
bool resetState = false;
unsigned long startMillis;
unsigned long currentMillis;
unsigned long elapsedMillis;

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
long initial_homing = -1;

Servo myservo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myservo.attach(servoPin);
  myservo.write(180);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  pinMode(homeSwitch, INPUT_PULLUP);
  pinMode(IRSensor, INPUT);
  goHome();
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

}

void loop() {
  // put your main code here, to run repeatedly:
  //Recieve is zero, not recieve is 1
//  int IRStatus = digitalRead(IRSensor);
//  Serial.println("IRStatus is: " + String(IRStatus));
  setSpeed_Acc();
  switch (Master_command){
      case 11: 
        startMillis = millis();
        Serial.println("r is called, move to r");
        stepper.moveTo(posR);
        stepper.setSpeed(1000);
        stepper.runToPosition();
        Serial.println(stepper.distanceToGo());
        if (stepper.distanceToGo() == 0){
          for (pos = 180; pos >= 155; pos -= 1) {
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          delay(5000);
          for (pos = 155; pos <= 180; pos += 1){ // goes from 180 degrees to 0 degrees
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          stepper.moveTo(0);
          stepper.setSpeed(-1000);
          stepper.runToPosition();
        }
        if (stepper.currentPosition() == 0){
          Master_command = 0;
          sortingDone = true;
          elapsedMillis = millis() - startMillis;
          stopWatch();
        }
        break;
      case 12:
        startMillis = millis();
        Serial.println("b is called, move to b");
        stepper.moveTo(posB);
        stepper.setSpeed(1000);
        stepper.runToPosition();
        Serial.println(stepper.distanceToGo());
        if (stepper.distanceToGo() == 0){
          for (pos = 180; pos >= 155; pos -= 1) {
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          delay(5000);
          for (pos = 155; pos <= 180; pos += 1){ // goes from 180 degrees to 0 degrees
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          stepper.moveTo(0);
          stepper.setSpeed(-1000);
          stepper.runToPosition();
        }
        if (stepper.currentPosition() == 0){
          Master_command = 0;
          sortingDone = true;
          elapsedMillis = millis() - startMillis;
          stopWatch();
        }
        break;
      case 13:
        startMillis = millis();
        Serial.println("y is called, move to y");
        stepper.moveTo(posY);
        stepper.setSpeed(1000);
        stepper.runToPosition();
        Serial.println(stepper.distanceToGo());
        if (stepper.distanceToGo() == 0){
          for (pos = 180; pos >= 155; pos -= 1) {
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          delay(5000);
          for (pos = 155; pos <= 180; pos += 1){ // goes from 180 degrees to 0 degrees
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          stepper.moveTo(0);
          stepper.setSpeed(-1000);
          stepper.runToPosition();
        }
        if (stepper.currentPosition() == 0){
          Master_command = 0;
          sortingDone = true;
          elapsedMillis = millis() - startMillis;
          stopWatch();
        }
        break;
      case 14:
        startMillis = millis();
        Serial.println("w is called, move to w");
        stepper.moveTo(posW);
        stepper.setSpeed(1000);
        stepper.runToPosition();
        Serial.println(stepper.distanceToGo());
        if (stepper.distanceToGo() == 0){
          for (pos = 180; pos >= 155; pos -= 1) {
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          delay(5000);
          for (pos = 155; pos <= 180; pos += 1){ // goes from 180 degrees to 0 degrees
            Serial.println(pos);
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
          }
          stepper.moveTo(0);
          stepper.setSpeed(-1000);
          stepper.runToPosition();
        }
        if (stepper.currentPosition() == 0){
          Master_command = 0;
          sortingDone = true;
          elapsedMillis = millis() - startMillis;
          stopWatch();
        }
        break;
      case 15:
//        Serial.println("r is called, move to r");
//        stepper.moveTo(1088);
//        stepper.setSpeed(1000);
//        stepper.runToPosition();
//        Serial.println(stepper.distanceToGo());
//        if (stepper.distanceToGo() == 0){
//          for (pos = 180; pos >= 135; pos -= 1) {
//            Serial.println(pos);
//            myservo.write(pos);              // tell servo to go to position in variable 'pos'
//            delay(15);                       // waits 15ms for the servo to reach the position
//          }
//          delay(5000);
//          for (pos = 135; pos <= 180; pos += 1){ // goes from 180 degrees to 0 degrees
//            Serial.println(pos);
//            myservo.write(pos);              // tell servo to go to position in variable 'pos'
//            delay(15);                       // waits 15ms for the servo to reach the position
//          }
//          stepper.moveTo(0);
//          stepper.setSpeed(-1000);
//          stepper.runToPosition();
//        }
//        if (stepper.currentPosition() == 0){
//          Master_command = 0;
//          sortingDone = true;
//        }
        break;
    }
}

void requestEvent(){
  if (Master_command == 3){
    Serial.println("Master tell me to check if receive object");
    //check if recieved object:
    if (digitalRead(IRSensor) == 0){
      Serial.println("Recieved object");
      Wire.write(1); //recieved the object, close the door, change the state
    }else{
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= Interval){
      previousMillis = currentMillis;
      timer--;
      Serial.println(timer);
      //never recieve object, but leave the door open, repeat state
      }
      if (timer == 0){
        Wire.write(2); //never recieve object, close the door, change the state to idle
        Serial.println("Sending master 2, never recieve object in time");
        timer = 30;
//        delay(10000);
        }else{
          Wire.write(0);
          Serial.println("Sending master 0, await object");
        }
      }
    }else if (Master_command == 4){
      Serial.println("Master tell me to go home and ask for colour");
      //first check if need to go home
      if (!isHome){
        goHome();
      }else{
        Serial.println("I am home"); 
        Wire.write(3); //tell master home is done, send colour information
        Serial.println("Sending master 3, tell him send me colour");
      }
    }else if (Master_command == 0){
      Serial.println("state of sorting: " + String(sortingDone));
      if (sortingDone){
        Serial.println("Sorting is done");
        Wire.write(4);
        sortingDone = false;
      }else if (!sortingDone){
        Serial.println("Sorting is not done");
        Wire.write(5);
      }
    }
}


void receiveEvent(){
  Master_command = Wire.read();
  Serial.println("Master Command: " + String(Master_command));
  }

void goHome(){
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
  Serial.print("Stepper is Homing . . . . . . . . . . . ");
  Serial.println(digitalRead(homeSwitch));
  while (digitalRead(homeSwitch)) {  // Make the Stepper move CCW until the switch is activated
    stepper.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepper.run();  // Start moving the stepper
    delay(5);
  }

  stepper.setCurrentPosition(0);  // Set the current position as zero for now
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
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
  stepper.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(1000.0);  // Set Acceleration of Stepper
}

void stopWatch(){
//    unsigned long seconds = (elapsedMillis%1000);
    unsigned long seconds = elapsedMillis/1000;
    Serial.print("Elapsed Time: ");
//    Serial.print(durSS);
//    Serial.print(" : ");
    Serial.print(seconds);
    Serial.print(" sec");
    Serial.println("");
}
