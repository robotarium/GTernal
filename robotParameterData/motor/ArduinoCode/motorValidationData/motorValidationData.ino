// Determine the Motor Characteristics for GRITSBOTX 

// PIN DEFINITIONS 

// Motor A(left) and B(right) are connected to AIN1,AIN2 and BIN1 and BIN2 on the motor controller 

#include <Encoder.h>

//////DEFINE ENCODER OBJECTS FOR LEFT AND RIGHT WHEEL //////////////
Encoder motorLeft(5, 6);
Encoder motorRight(21,20);

//  Pin Definitions 
int STBY = 4; // motor driver enable 
int led = 13; // Teensy LED Pin 

// Motor A 
int PWMA = 9; // control the speed 
int AIN1 = 0; // direction 
int AIN2 = 1; // direction 

//Motor B 
int PWMB = 10; // speed control 
int BIN1 = 2; // direction 
int BIN2 = 3; // direction 


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

float samplingTime = 5; //10 ms sampling time.

//PID Stuff
float PIDMotorsTimeStart = 0;
float oldErrorL = 0;
float oldErrorR = 0;
float oldMotorPIDEncoderCountL = 0;
float oldMotorPIDEncoderCountR = 0;
float encoderCountsPerRotation = 12;
float motorGearRatio = 150.58;
float kpMotor = 6.01;
float kiMotor = 613;
float kdMotor = 0;
int motorL = 0;
int motorR = 0;

float desVel[5] = {random(-8,8), random(-8,8), random(-8,8), random(-8,8), random(-8,8)};//rad/s

int input = 0;

////////////////////// SETUP //////////////////////////////////
void setup() {
  // define the pins as outputs
  pinMode(STBY,OUTPUT);
  pinMode(led,OUTPUT);
  
  pinMode(PWMA,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);

  pinMode(PWMB,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);

  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second

  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  delay(1000);//Allow Time for Matlab to start.
  PIDMotorsTimeStart = millis();
}

//////////////// MAIN LOOP ////////////////////////////////////

void loop()                     
{ 
  for (int i = 0; i<5; i++){
    if (i == 0){
      timeStart = millis();
    }
    timeStartStep=millis();
    while (millis() - timeStartStep < stepExperimentTime){
      PIDMotorControl(desVel[i],desVel[i]);
      if (millis() - timeStart >= samplingTime){
        readEncoders();
        timeStart = millis();
        //Correct for wrap around
        if (countL < 0 && oldCountL > 0){
          countL = ((countL - (-2147483648)) + (2147483647 - oldCountL));
        }
        if (countR < 0 && oldCountR > 0){
          countR = ((countR - (-2147483648)) + (2147483647 - oldCountR)); // (rad)
        }
        oldCountR = countR;
        oldCountL = countL;
        Serial.print(int(countL));
        Serial.print(",");
        Serial.print(int(countR));
        Serial.print(",");
        Serial.print(int(millis()));
        Serial.print(",");
        Serial.println(desVel[i]);    
        Serial.flush();
       }
    }
  }
  

   stop();
   Serial.flush();
   Serial.print("Done");
   Serial.end();
   while(1){
      digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);               // wait for a second
      digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);               // wait for a second
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

void PIDMotorControl(float desLVel, float desRVel){
    /*Keeps the rotational speeds of the individual motors at setpoints desLVel and desRVel (rad/s).*/

    float timeStep = 10;

    if (millis() - PIDMotorsTimeStart >= timeStep){
      float PIDTimeStep = (millis() - PIDMotorsTimeStart)/1000;//Time step for controller to work on (s).

      readEncoders();

      // Error on individual motors for vel control
      float errorL = desLVel - 2 * pi * (countL - oldMotorPIDEncoderCountL) / (encoderCountsPerRotation * motorGearRatio * PIDTimeStep);
      float errorR = desRVel - 2 * pi * (countR - oldMotorPIDEncoderCountR) / (encoderCountsPerRotation * motorGearRatio * PIDTimeStep);
      float integralL = integralL + errorL * PIDTimeStep;
      float integralR = integralR + errorR * PIDTimeStep;
      float diffL = (oldErrorL - errorL) / PIDTimeStep;
      float diffR = (oldErrorR - errorR) / PIDTimeStep;
      oldErrorL = errorL;
      oldErrorR = errorR;
      oldMotorPIDEncoderCountL = countL;
      oldMotorPIDEncoderCountR = countR;

      motorL += int((kpMotor*errorL + kiMotor*integralL + kdMotor*diffL));
      motorR += int((kpMotor*errorR + kiMotor*integralR + kdMotor*diffR));

      moveL(motorL);
      moveR(motorR);
      
      PIDMotorsTimeStart = millis();
    }
}

