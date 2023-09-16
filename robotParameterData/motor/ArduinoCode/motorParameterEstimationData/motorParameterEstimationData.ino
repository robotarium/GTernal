// Determine the Motor Characteristics for GRITSBOTX 

// PIN DEFINITIONS 

// Motor A(left) and B(right) are connected to AIN1,AIN2 and BIN1 and BIN2 on the motor controller 

#include <Encoder.h>

//////DEFINE ENCODER OBJECTS FOR LEFT AND RIGHT WHEEL //////////////
Encoder motorLeft(20, 21);
Encoder motorRight(2,3);

//  Pin Definitions 
int led = 13; // Teensy LED Pin 

//Constants

float pi = 3.14159265359;

//Encoder Count Storage
int countR = 0; // Current count for RIGHT encoder.
int countL = 0; // Current count for LEFT encoder.

int oldCountR = 0;
int oldCountL = 0;

//Other Variables
float timeStart=0;
float timeStartStep=0;

float experimentTime = 60; //60 seconds.
float stepExperimentTime = 2000; // 2 Seconds in ms.

float samplingTime = 10; //10 ms sampling time.

////////////////////// SETUP //////////////////////////////////
void setup() {
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second

  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  delay(1000);//Allow Time for Matlab to start.
}

//////////////// MAIN LOOP ////////////////////////////////////

void loop()                     
{ 
  for (int i=1; i<100; i++){
    if (i == 1){
      timeStart = millis();
    }
      if (millis() - timeStart >= samplingTime){
        readEncoders();

        //Correct for wrap around
        if (countL < 0 && oldCountL > 0){
          countL = ((countL - (-2147483648)) + (2147483647 - oldCountL));
        }
        if (countR < 0 && oldCountR > 0){
          countR = ((countR - (-2147483648)) + (2147483647 - oldCountR)); // (rad)
        }
        Serial.print(int(countL));
        Serial.print(",");
        Serial.print(int(countR));
        Serial.print(",");
        Serial.print(int(millis()));
        Serial.print(",");
        Serial.println(input);
        timeStart = millis();
      }
    }
}
////////////////// FUNCTIONS //////////////////////////////////
void moveR(int motorSpeed){
  //Assume forward at first
  boolean in1 = HIGH;
  boolean in2 = LOW;

  //If motor speed is negative we want to rotate wheel backwards
  if (motorSpeed < 0){
     in1 = LOW;
     in2 = HIGH;
  }
   
  // Truncate PWM input to 255
  if (abs(motorSpeed)>255){
    motorSpeed = 255;
  }

  // Move the motor.
  digitalWrite(STBY, HIGH);
  digitalWrite(BIN1,in1);
  digitalWrite(BIN2,in2);
  analogWrite(PWMB, abs(motorSpeed)); 
}

void moveL(int motorSpeed){
  //Assume forward at first
  boolean in1 = LOW;
  boolean in2 = HIGH;

  //If motor speed is negative we want to rotate wheel backwards
  if (motorSpeed < 0){
     in1 = HIGH;
     in2 = LOW;
  }
   
  // Truncate PWM input to 255
  if (abs(motorSpeed)>255){
    motorSpeed = 255;
  }

  // Move the motor.
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1,in1);
  digitalWrite(AIN2,in2);
  analogWrite(PWMA, abs(motorSpeed));
}

void brake(){
  //Brakes the motors. (Locked to not rotate passively, this can be overcome by enough torque)
  digitalWrite(STBY, HIGH);
  
  analogWrite(PWMA, LOW);
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  
  analogWrite(PWMB, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, HIGH);
}

//Positive speed drives straight
void driveStraight(int motorSpeed){
  moveL(motorSpeed);
  moveR(motorSpeed);
}

//Positive speed turns right
void turn(int motorSpeed){
  moveL(motorSpeed);
  moveR(-motorSpeed);
}

void stop() {
  // Turns the motors off. (They can still rotate passively) 
  digitalWrite(STBY,LOW);
}

void readEncoders(){
  countR = -motorRight.read();
  countL = motorLeft.read();
}

