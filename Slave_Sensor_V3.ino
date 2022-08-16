#include "HX711.h"
#include <Wire.h>
#include <Servo.h>

#define calibration_factor  754//This value is obtained using the SparkFun_HX711_Calibration sketch
#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2
#define SLAVE_ADDR 8
#define servoPin 6
#define PWM_PIN 5

// load cell readings
float weightResult;
// byteArray for wire
byte byteArray[2];  //split 16 bits into 2 bytes
byte byteColorArray[3];
int Master_command;
int color;
unsigned pwm_value;
int pwm_ls[8][2] = {{200,4000},{400,8000},{400,8000},{500,12000},{600,12000},{700,16000},{800,16000},{900,18000}};
int pwm_switch = 0;

HX711 scale;
Servo myservo;

void setup() {
  Serial.begin(9600);
  pinMode(PWM_PIN, INPUT); 
  
  //start the I2C Bus as Slave on address 8 (Address can be from 0-127)
  Wire.begin(SLAVE_ADDR);
  // Function to run when data requested from master
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  //set-up for weight sensor
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  // Servo setup
  myservo.attach(servoPin);
  myservo.write(0);
  
}

void loop() {
  delay(50);
}

void requestEvent() {
  if (Master_command == 0) {   
    Serial.println("Master tell me to start weighing");
    Serial.print("Reading: ");
    weightResult = scale.get_units(5); //scale.get_units() returns a int
    Serial.print(weightResult);
    Serial.print(" g"); 
    Serial.println();
    if (weightResult < 0.10){
      weightResult = 0; 
    }
    union weight_tag {
      char weight_b[4];
      float weight_2send;
    } weight_Union;
    weight_Union.weight_2send = weightResult;
    Wire.write(weight_Union.weight_b,4);
  }else if (Master_command == 1) {
    Serial.println("Master tell me to start color sensing");
    pwm_value = pulseIn(PWM_PIN, HIGH);
    delay(700);
    Serial.println(pwm_value);
    if (pwm_value > 1000){
      pwm_switch = 1;
    }else{
      pwm_switch = 0;
    }
    if (pwm_value > pwm_ls[0][pwm_switch] && pwm_value < pwm_ls[1][pwm_switch]){ // 200 to 300 or btw 4000 - 5000
      Serial.println("Color is Red");
      color = 1;
    }
    else if (pwm_value > pwm_ls[2][pwm_switch] && pwm_value < pwm_ls[3][pwm_switch]){ //400 to 500 or btw 8000 to 9000
      Serial.println("Color is Blue");
      color = 2;
    }
    else if (pwm_value > pwm_ls[4][pwm_switch] && pwm_value < pwm_ls[5][pwm_switch]){ //600 to 700 or btw 12000 to 13000
      Serial.println("Color is Yellow");
      color = 3;
    }
    else if (pwm_value > pwm_ls[6][pwm_switch] && pwm_value < pwm_ls[7][pwm_switch]){ //800 to 900 or 16000 to 17000
      Serial.println("Color is White");
      color = 4;
    }else{
      Serial.println("No or unknown color detected");
      color = 0;
    }
    Wire.write(byte(color)); // Send Color data as byte data type via I2C
  
   }else{
    Serial.println("Master inistructions unclear");
  }
}

void receiveEvent() {
  Serial.println("I hear Master");
  Master_command = Wire.read();
  Serial.println("Master Command: " + String(Master_command));
  if (Master_command == 2){
    Serial.println("Time to move servo");
    myservo.write(90);
  }else if (Master_command == 3){
    Serial.println("Servo move, close door");
    myservo.write(0);
  }
}

// int2byte() convert weight int data to byte array
// float2byte(float floatData){
//  union weight_tag {
//  char weight_b[4];
//  float weight_2send;
//  } weight_Union;
//  weight_Union.weight_2send = floatData;
//  return weight_U 
//}
