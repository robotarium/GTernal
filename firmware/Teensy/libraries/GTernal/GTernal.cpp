#include "GTernal.h"

Adafruit_INA260 ina260 = Adafruit_INA260();
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;
VL53L4CD sensor_vl53l4cd_sat(&Wire, A1);
int NUMBER_OF_SENSORS = 6;  // Number of VL53L4CD sensors on GTernal. N_sensors - 1 for indexing purposes.

// Initialize static members
GTernal* GTernal::_instance = nullptr;
IntervalTimer GTernal::timerISR;

GTernal::GTernal():_motorLeft(_interruptL1, _interruptL2),_motorRight(_interruptR1, _interruptR2), \
_strip(2,10,NEO_GRBW + NEO_KHZ800){
  _instance = this; // Set the instance pointer to this object instance
}

///////////////////////////////////////////////////////////
//Setup
///////////////////////////////////////////////////////////

void GTernal::SETUP(){

  ///////////////////////////////////////////////////////////
  //Set Pin Modes!
  ///////////////////////////////////////////////////////////

  //Battery Feedback Pins
  pinMode(_battChargingCheck, INPUT_PULLDOWN);//Set the Battery Charging Pin to Input,
  // use pulldown so when the charger is not connected the pin stays low.

  //Motor Control 
  pinMode(_STBY, OUTPUT);
  pinMode(_motEN, OUTPUT);
  digitalWrite(_motEN, HIGH); //Enable the motor driver

  pinMode(_PWMR, OUTPUT); 
  pinMode(_PWML, OUTPUT);  
  
  pinMode(_RMotor1, OUTPUT); 
  pinMode(_RMotor2, OUTPUT); 
  
  pinMode(_LMotor1, OUTPUT); 
  pinMode(_LMotor2, OUTPUT);

  //RPi Enable Pin
  pinMode(_rpiEnable, OUTPUT);
  digitalWrite(_rpiEnable, LOW);

  // Initialize I2C bus.
  Wire.begin();

  ///////////////////////////////////////////////////////////
  // Neo Pixel Setup
  ///////////////////////////////////////////////////////////
  _strip.begin(); // Initialize the neo pixel library
  _strip.setBrightness(64); // Set LEDs to 1/4 brightness.
  _strip.show(); // Turn LEDs off (initially set to off)

  ///////////////////////////////////////////////////////////
  // INA260 Setup
  ///////////////////////////////////////////////////////////
  // if (!ina260.begin())
  // {
  //   Serial.print("Ooops, no INA260 detected ... Check your wiring or I2C ADDR!");
  //   while (1);
  // }

  ///////////////////////////////////////////////////////////
  // VL53L4CD Setup
  ///////////////////////////////////////////////////////////
  // Configure VL53L4CD satellite component.
  // for (byte x = 0 ; x <= NUMBER_OF_SENSORS ; x++)
  // {
  //   enableMuxPort(x); //Tell mux to connect to port X
  //   sensor_vl53l4cd_sat.begin();
  //   //Switch off VL53L4CD satellite component.
  //   sensor_vl53l4cd_sat.VL53L4CD_Off();
  //   //Initialize VL53L4CD satellite component.
  //   sensor_vl53l4cd_sat.InitSensor();
  //   // 10ms timing budget and continuous mode
  //   sensor_vl53l4cd_sat.VL53L4CD_SetRangeTiming(10, 0);
  //   // Start Measurements
  //   sensor_vl53l4cd_sat.VL53L4CD_StartRanging();
  //   // Serial.println(int(x));
  // }

  // ///////////////////////////////////////////////////////////
  // // BNO055 Setup
  // ///////////////////////////////////////////////////////////
  // Initialize the sensor
  // if (!bno.begin())
  // {
  //   /* There was a problem detecting the BNO055 ... check your connections */
  //   Serial.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  //   while (1);
  // }
  // Wire.endTransmission();

  ///////////////////////////////////////////////////////////
  //Start Timers for Control Loops
  ///////////////////////////////////////////////////////////
  _driveStart = false;
  _PIDMotorsTimeStart = millis();
  _positionUpdateTimeStart = millis();
  _communicationTimeout = millis();
}

///////////////////////////////////////////////////////////
//Timer Interrupt Functions
///////////////////////////////////////////////////////////

void GTernal::beginISR(){
  timerISR.begin(isrHandler, 10000); // 10000 us = 10 ms
}

void GTernal::isrHandler(){
    if (_instance){
      _instance -> followCommands();
    }
}

///////////////////////////////////////////////////////////
//Conversion Functions
///////////////////////////////////////////////////////////

float GTernal::_deg2rad(float deg){
  //Converts degree to radians
  float output = deg * _pi / 180;
  return output;
}

float GTernal::_rad2deg(float rad){
  //Converts radians to degree
  float output = rad * 180 / _pi;
  return output;
}

float GTernal::_wrapToPi(float rad){
  //Converts radian angle (-pi,pi]
  float output = atan2(sin(rad),cos(rad));
  return output;
}

float GTernal::_wrapTo2Pi(float rad){
  //Converts radian angle [0,2*pi)
  float output = _wrapToPi(rad);
  if (output < 0){
    output = output + 2*_pi;
  }
  return output;
}

///////////////////////////////////////////////////////////
// Unicycle Model Conversions
///////////////////////////////////////////////////////////

float GTernal::convertUnicycleToRightMotor(float vel, float w){
  float output = (2.0*vel+w*_axelLength)/(_wheelDiameter);
  return output;
}

float GTernal::convertUnicycleToLeftMotor(float vel, float w){
  float output = (2.0*vel-w*_axelLength)/(_wheelDiameter);
  return output;
}

///////////////////////////////////////////////////////////
// JSON Serial Messaging
///////////////////////////////////////////////////////////

void GTernal::jsonSerialRead(){
  Serial3.clear();
  if(Serial3.available()){
    JsonObject& jsonIn = _jsonBufferIn.parseObject(Serial3);
    JsonObject& jsonOut = _jsonBufferOut.createObject();
    JsonArray& statusArray = jsonOut.createNestedArray("status");
    JsonArray& bodyArray = jsonOut.createNestedArray("body");
    JsonArray& requestArray = jsonIn["request"];
    JsonArray& ifaceArray = jsonIn["iface"];
    int requestArraySize = requestArray.size();//Same number of requests as iface.
    for(int i=0;i<requestArraySize;i++){
      const char* requestStr = requestArray[i];
      if(strcmp(requestStr,"read") == 0){
        _method = 0;
      }
      else if(strcmp(requestStr,"write") == 0){
        _method = 1;
      }
      switch(_method){
        case 0://read
        {
          JsonObject& body = bodyArray.createNestedObject();
          const char* ifaceStr = ifaceArray[i];
          if (strcmp(ifaceStr,"batt_volt") == 0){
            statusArray.add(1);
            body["batt_volt"] = checkBattVoltage();
          }
          else if (strcmp(ifaceStr,"charge_status") == 0){
            statusArray.add(1);
            body["charge_status"] = checkCharging();
          }  
          else if (strcmp(ifaceStr,"bus_volt") == 0){
            statusArray.add(1);
            body["bus_volt"] = ina260.readBusVoltage();
            // body["bus_volt"] = filteredMeasurement(20, "voltage");
          }
          else if (strcmp(ifaceStr,"bus_current") == 0){
            statusArray.add(1);
            body["bus_current"] = ina260.readCurrent();
            // body["bus_current"] = filteredMeasurement(20, "current");
          }
          else if (strcmp(ifaceStr,"power") == 0){
            statusArray.add(1);
            body["power"] = ina260.readPower();
            // body["power"] = filteredMeasurement(20, "power");
          }
          else if (strcmp(ifaceStr,"distances") == 0){ 
            statusArray.add(1);
            JsonArray& distanceArray = body.createNestedArray("distances");
            measureDistances(distanceArray);
          } 
          else if (strcmp(ifaceStr,"orientation") == 0){
            statusArray.add(1);
            JsonArray& orientationArray = body.createNestedArray("orientation");
            getOrientation(orientationArray);
          }
          break;
        }
        case 1://write
        {
          JsonObject& body = bodyArray.createNestedObject();
          const char* ifaceStr = ifaceArray[i];
          if (strcmp(ifaceStr,"motor") == 0){
            statusArray.add(1);
            noInterrupts(); // Critical section
            _v = jsonIn["body"][i]["v"];            
            _w = jsonIn["body"][i]["w"];
            interrupts();
          }
          else if (strcmp(ifaceStr,"left_led") == 0){
            statusArray.add(1);
            JsonArray& _leftLED = jsonIn["body"][i]["rgb"];//Left LED RGB value
            setSingleLED(1,_leftLED[0],_leftLED[1],_leftLED[2],0);
          }
          else if (strcmp(ifaceStr,"right_led") == 0){
            statusArray.add(1);
            JsonArray& _rightLED = jsonIn["body"][i]["rgb"];//Right LED RGB value
            setSingleLED(0,_rightLED[0],_rightLED[1],_rightLED[2],0);
          }
          else if (strcmp(ifaceStr,"fast_charge") == 0){
            Serial.println("Entering fast charging mode"); // For debugging
            statusArray.add(1);
            setFastChargingFlag(1);
          }

          _communicationTimeout = millis();
          break;
        }
      }  
    }
    if(Serial3.availableForWrite() >= jsonOut.size()+1){
      jsonOut.printTo(Serial3);
      // jsonOut.printTo(Serial); // For debugging
      // Serial.println(jsonOut.size()); // For debugging
      _jsonBufferOut.clear();
    }
    _jsonBufferIn.clear();
  }
}

void GTernal::communicationCheck(float communicationWaitTime){
  if (millis() - _communicationTimeout > communicationWaitTime){
    brake();
    noInterrupts(); // Critical section
    _v=0;
    _w=0;
    interrupts();
  }
}

void GTernal::followCommands(){
  if (abs(_v) < float(0.0001) and abs(_w) < float(0.01)){
    noMotion();
  }
  else{
    PIDMotorControl(convertUnicycleToLeftMotor(_v,_w),convertUnicycleToRightMotor(_v,_w));
  }
}

///////////////////////////////////////////////////////////
//Neo Pixel Functions
///////////////////////////////////////////////////////////

void GTernal::setSingleLED(int pos, int r, int g, int b, int w){
  //Sets the color of one LED
  if (pos >= _strip.numPixels()){ // Controlling a non-existant LED (as defined in SETUP).
    return;
  } 
  _strip.setPixelColor(pos, r, g, b, w);
  _strip.show();
}

void GTernal::setAllLED(int r, int g, int b, int w){
  //Sets the color of both LEDs
  for (int i = 0; i < _strip.numPixels(); i++){
    _strip.setPixelColor(i, r, g, b, w);
  }
  _strip.show();
}

void GTernal::turnOffLED(){
  //Turns off both LEDs
  for (int i = 0; i < _strip.numPixels(); i++){
    _strip.setPixelColor(i, 0, 0, 0, 0);
  }
  _strip.show();
}

void GTernal::rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<_strip.numPixels(); i++) {
      // _strip.setPixelColor(i, _wheel((i+j) & 255));
      _strip.setPixelColor(i, _wheel((i+j)));
    }
    _strip.show();
    delay(wait);
  }

  // Fade in and fade out the white LED on the Neopixel
  for (int k = 0; k < 255; k++){
    _strip.setPixelColor(0, 0, 0, 0, k);
    // _strip.setPixelColor(1, 0, 0, 0, k);
    _strip.show();
    delay(wait/2);
  }

  for (int k = 255; k > 0; k--){
    _strip.setPixelColor(0, 0, 0, 0, k);
    // _strip.setPixelColor(1, 0, 0, 0, k);
    _strip.show();
    delay(wait/2);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t GTernal::_wheel(byte wheelPos) {
  wheelPos = 255 - wheelPos;
  if(wheelPos < 85) {
    return _strip.Color(255 - wheelPos * 3, 0, wheelPos * 3, 0);
  }
  if(wheelPos < 170) {
    wheelPos -= 85;
    return _strip.Color(0, wheelPos * 3, 255 - wheelPos * 3, 0);
  }
  wheelPos -= 170;
  return _strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0, 0);
}

///////////////////////////////////////////////////////////
//Read Sensors
///////////////////////////////////////////////////////////

bool GTernal::checkCharging(){
  //Determines if the robot is charging its battery
  bool chargingStatus = digitalRead(_battChargingCheck);
  return chargingStatus;
}

float GTernal::checkBattVoltage(){
  //Determines if the robot is charging its battery
  float input = analogRead(_battVoltage);

  //Scale the reading from the voltage divider to determine the correct reading.
  float battVoltage = input * 3.3/1023.0 * ((5.0+15.0)/15.0);

  // Read battery voltage from INA260
  // float battVoltage = ina260.readBusVoltage();
  return battVoltage;
}

void GTernal::_readEncoders(){
  // Reads the encoder and returns the current count.
  _encoderCountR = -_motorRight.read();
  _encoderCountL = _motorLeft.read();
}

void GTernal::getEncoderCounts(int encoderData[]){
  // Reads the current encoder count and stores it in the array argument. [Left, Right]
  _readEncoders();
  encoderData[0] = _encoderCountL;
  encoderData[1] = _encoderCountR;
}

// Mux selection function for VL53L4CD sensors
void GTernal::enableMuxPort(uint8_t port){
  Wire.beginTransmission(0x70); //select the mux
  Wire.write(1 << port);        //send bit to select bus
  Wire.endTransmission();
}

void GTernal::measureDistances(JsonArray& outputArray){
  array<VL53L4CD_Result_t, 7> sensorOutputs;
  
  for(byte x = 0 ; x <= NUMBER_OF_SENSORS ; x++){
      // (Mandatory) Clear HW interrupt to restart measurements
      enableMuxPort(x); //Tell mux to connect to port X
      sensor_vl53l4cd_sat.VL53L4CD_ClearInterrupt();
      // Read measured distance. RangeStatus = 0 means valid data
      sensor_vl53l4cd_sat.VL53L4CD_GetResult(&sensorOutputs[x]);
      outputArray.add(double(sensorOutputs[x].distance_mm)/1000); // Store distance value in meters
  }
}

void GTernal::getOrientation(JsonArray& oriOutputArray){
  sensors_event_t orientationData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  oriOutputArray.add(orientationData.orientation.x); // Store x, y, z orientation data
  oriOutputArray.add(orientationData.orientation.y);
  oriOutputArray.add(orientationData.orientation.z);
}

///////////////////////////////////////////////////////////
//Estimators
///////////////////////////////////////////////////////////

void GTernal::getGlobalPosition(float state[]){ //Returns the global position of the robot [x,y,theta].
  state[0] = _botXPos;
  state[1] = _botYPos;
  state[2] = _botA;
}

void GTernal::setGlobalPosition(float x, float y, float a){ // Set the global state of the robot.
  _botXPos = x;
  _botYPos = y;
  _botA = a;
}

void GTernal::encoderPositionUpdate(float timeStep){

  if (millis() - _positionUpdateTimeStart >= timeStep){
    timeStep = (millis() - _positionUpdateTimeStart)/1000; //Convert ms to s
    _positionUpdateTimeStart = millis();

    _readEncoders();
    int countL = _encoderCountL;
    int countR = _encoderCountR;

    float Dl = _pi*_wheelDiameter * (countL - _oldEPUEncoderCountL) / (_encoderCountsPerRotation * _motorGearRatio); // Linear distance left wheel has rotated.
    float Dr = _pi*_wheelDiameter * (countR - _oldEPUEncoderCountR) / (_encoderCountsPerRotation * _motorGearRatio); // Linear distance left wheel has rotated.
    //Check integer roll over!
    if (countL < 0 && _oldEPUEncoderCountL > 0){
      Dl = _pi*_wheelDiameter * ((countL - (-32768)) + (32767 - _oldEPUEncoderCountL)) / (_encoderCountsPerRotation * _motorGearRatio); // Linear distance left wheel has rotated.
    }
    if (countR < 0 && _oldEPUEncoderCountR > 0){
      Dr = _pi*_wheelDiameter * ((countR - (-32768)) + (32767 - _oldEPUEncoderCountR)) / (_encoderCountsPerRotation * _motorGearRatio); // Linear distance left wheel has rotated.
    }
    float Dc = (Dr + Dl)/2; //Distance center of bot has moved read by encoders.
    _oldEPUEncoderCountR = countR;
    _oldEPUEncoderCountL = countL;
    
    _botA += (Dr - Dl)/_axelLength;

    _botXPos += Dc * cos(_botA0 + (_botA-_botA0)/2);
    _botYPos += Dc * sin(_botA0 + (_botA-_botA0)/2);

    _botVel = (Dr + Dl)/(2*timeStep);
    _botA0 = _botA;
  }  
}

void GTernal::getWheelSpeeds(float wheelSpeeds[]){
  //Returns the current wheel speeds of the robot [Left, Right].
  noInterrupts(); // Critical section
  wheelSpeeds[0] = _wheelSpeedL;
  wheelSpeeds[1] = _wheelSpeedR;
  interrupts();
}

///////////////////////////////////////////////////////////
//Motor Functions
///////////////////////////////////////////////////////////

void GTernal::noMotion(){
  //Stop the motor board from supplying current to motor.
  digitalWrite(_STBY, LOW);
  _readEncoders();

  // Make sure PID controller variables reset (Integral windup, time step being large).
  _driveStart = false;
}

void GTernal::brake(){
  //Brakes the motors. (Locked to not rotate passively, this can be overcome by enough torque)
  digitalWrite(_STBY, HIGH);

  analogWrite(_PWMR, LOW);
  digitalWrite(_RMotor1, HIGH);
  digitalWrite(_RMotor2, HIGH);
  
  analogWrite(_PWML, LOW);
  digitalWrite(_LMotor1, HIGH);
  digitalWrite(_LMotor2, HIGH);


  _readEncoders();

  // Make sure PID controller variables reset (Integral windup, time step being large).
  _driveStart = false;
}

void GTernal::moveR(int motorSpeed){
  digitalWrite(_STBY, HIGH);

  //Assume forward at first
  bool in1 = HIGH;
  bool in2 = LOW;

  //If motor speed is negative we want to rotate wheel backwards
  if (motorSpeed < 0){
     in1 = LOW;
     in2 = HIGH;
  }
   
  // Truncate PWM input to 255 in case this is not done previously
  if (abs(motorSpeed)>255){
    motorSpeed = 255;
  }

  // Move the motor.
  digitalWrite(_RMotor1, in1);
  digitalWrite(_RMotor2, in2);
  analogWrite(_PWMR, abs(motorSpeed)); 
}

void GTernal::moveL(int motorSpeed){
  digitalWrite(_STBY, HIGH);

  //Assume forward at first
  bool in1 = LOW;
  bool in2 = HIGH;

  //If motor speed is negative we want to rotate wheel backwards
  if (motorSpeed < 0){
     in1 = HIGH;
     in2 = LOW;
  }
   
  // Truncate PWM input to 255 in case this is not done previously
  if (abs(motorSpeed)>255){
    motorSpeed = 255;
  }

  // Move the motor.
  digitalWrite(_LMotor1, in1);
  digitalWrite(_LMotor2, in2);
  analogWrite(_PWML, abs(motorSpeed));
}

void GTernal::setVelocity(float v, float w){
  noInterrupts(); // Critical section
  _v = v;
  _w = w;
  interrupts();
}

///////////////////////////////////////////////////////////
//Controllers
///////////////////////////////////////////////////////////

void GTernal::PIDMotorControl(float desLVelInput, float desRVelInput){
    /*Keeps the rotational speeds of the individual motors at setpoints desLVel and desRVel (rad/s).*/

    float timeStep = 10;

    //Prefilter
    // float a = 0.18;//Slightly tweaked to lower overshoot.

    if(!_driveStart){//We recently stopped motion and didn't call the PID loop, reset the PID timers/variables.
      _driveStart = true;
      _PIDMotorsTimeStart = millis();
      _oldMotorPIDEncoderCountL = _encoderCountL;
      _oldMotorPIDEncoderCountR = _encoderCountR;
      _integralL = 0;
      _integralR = 0; 
      _oldErrorL = 0;
      _oldErrorR = 0;
      _oldMotorL = 0;
      _oldMotorR = 0;
    }

    if(millis() - _PIDMotorsTimeStart >= 3*timeStep){//There's been a large time delay, reset the timer and current encoder error
      _PIDMotorsTimeStart = millis();                //to avoide integral windup.
      _oldMotorPIDEncoderCountL = _encoderCountL;
      _oldMotorPIDEncoderCountR = _encoderCountR;
    }

    if (millis() - _PIDMotorsTimeStart >= timeStep){
      // _desVelR = (1-a)*_desVelR+a*desRVelInput;
      // _desVelL = (1-a)*_desVelL+a*desLVelInput;
      // float desLVel = _desVelL;
      // float desRVel = _desVelR;
      float desLVel = desLVelInput;
      float desRVel = desRVelInput;
      // float PIDTimeStep = (millis() - _PIDMotorsTimeStart)/1000;//Time step for controller to work on (s).
      float PIDTimeStep = 0.01;

      _readEncoders();
      int countL = _encoderCountL;
      int countR = _encoderCountR;

      // Error on individual motors for vel control
      float errorL = desLVel - 2.0 * _pi * (countL - _oldMotorPIDEncoderCountL) / (_encoderCountsPerRotation * _motorGearRatio * PIDTimeStep);
      float errorR = desRVel - 2.0 * _pi * (countR - _oldMotorPIDEncoderCountR) / (_encoderCountsPerRotation * _motorGearRatio * PIDTimeStep);
      
      // Check and correct for rollover
      if (countL < 0 && _oldMotorPIDEncoderCountL > 0 && _oldMotorPIDEncoderCountL > 20000){
        errorL = desLVel - 2.0 * _pi * ((countL - (-32768)) - (32767 - _oldMotorPIDEncoderCountL)) / (_encoderCountsPerRotation * _motorGearRatio * PIDTimeStep);
      }
      if (countL > 0 && _oldMotorPIDEncoderCountL < 0 && _oldMotorPIDEncoderCountL < -20000){
        errorL = desLVel - 2.0 * _pi * ((32767 - countL) - (_oldMotorPIDEncoderCountL - (-32768))) / (_encoderCountsPerRotation * _motorGearRatio * PIDTimeStep);
      }

      if (countR < 0 && _oldMotorPIDEncoderCountR > 0 && _oldMotorPIDEncoderCountR > 20000){
        errorR = desRVel - 2.0 * _pi * ((countR - (-32768)) - (32767 - _oldMotorPIDEncoderCountR)) / (_encoderCountsPerRotation * _motorGearRatio * PIDTimeStep);
      }
      if (countR > 0 && _oldMotorPIDEncoderCountR < 0 && _oldMotorPIDEncoderCountR < -20000){
        errorR = desRVel - 2.0 * _pi * ((32767 - countR) - (_oldMotorPIDEncoderCountR - (-32768))) / (_encoderCountsPerRotation * _motorGearRatio * PIDTimeStep);
      }

      _integralL = _integralL + errorL * PIDTimeStep;
      _integralR = _integralR + errorR * PIDTimeStep;
      // float diffL = (_oldErrorL - errorL) / PIDTimeStep;
      // float diffR = (_oldErrorR - errorR) / PIDTimeStep;
      float diffL = (errorL - _oldErrorL) / PIDTimeStep;
      float diffR = (errorR - _oldErrorR) / PIDTimeStep;
      _oldErrorL = errorL;
      _oldErrorR = errorR;
      _oldMotorPIDEncoderCountL = countL;
      _oldMotorPIDEncoderCountR = countR;

      _wheelSpeedL = desLVel - errorL;
      _wheelSpeedR = desRVel - errorR;


      //Get rid of integral windup with feedback loop.
      if(_satR){
        _motorR += int(_motorDigitalK*(_kpMotor*errorR + _kiMotor*(_integralR + _satRVal*PIDTimeStep) + _kdMotor*diffR));
        _satR = false;
      }
      else{
        _motorR += int(_motorDigitalK*(_kpMotor*errorR + _kiMotor*_integralR + _kdMotor*diffR));
      }

      if(_satL){
        _motorL += int(_motorDigitalK*(_kpMotor*errorL + _kiMotor*(_integralL + _satLVal*PIDTimeStep) + _kdMotor*diffL));
        _satL = false;
      }
      else{
        _motorL += int(_motorDigitalK*(_kpMotor*errorL + _kiMotor*_integralL + _kdMotor*diffL));
      }

      if (abs(_motorL - _oldMotorL) > _maxMotorInc){ // Check for command that requires high acceleration
        if(_motorL - _oldMotorL > 0){
          _motorL = _oldMotorL + _maxMotorInc;
        }else{
          _motorL = _oldMotorL - _maxMotorInc;
        }
        
      }

      if (abs(_motorR - _oldMotorR) > _maxMotorInc){ // Check for command that requires high acceleration
        if(_motorR - _oldMotorR > 0){
          _motorR = _oldMotorR + _maxMotorInc;
        }else{
          _motorR = _oldMotorR - _maxMotorInc;
        }
      }

      //Check and deal with motor saturation.
      if (_motorL>255){
        _integralL -= (errorL*PIDTimeStep); //Don't integrate at saturation
        _satLVal = (255 - _motorL);
        _satL = true;
        _motorL=255;
      }
      if (_motorR>255){
        _integralR -= (errorR*PIDTimeStep); // Don't integrate at saturation
        _satRVal = (255 - _motorR);
        _satR = true;
        _motorR=255;
      }
      if (_motorL<-255){
        _integralL -= (errorL*PIDTimeStep); // Don't integrate at saturation
        _satLVal = (-255 - _motorL);
        _satL = true;
        _motorL=-255;
      }
      if (_motorR<-255){
        _integralR -= (errorR*PIDTimeStep); // Don't integrate at saturation
        _satRVal = (-255 - _motorR);
        _satR = true;
        _motorR=-255;
      }

      _oldMotorR = _motorR;
      _oldMotorL = _motorL;

      moveL(_motorL);
      moveR(_motorR);

      _PIDMotorsTimeStart = millis();
    }
}

///////////////////////////////////////////////////////////
//Fast Charging Mode
///////////////////////////////////////////////////////////
void GTernal::setFastChargingFlag(bool flag){
  _flagFastCharging = flag;

  if (_flagFastCharging){
    resetVoltageArray();
    setSingleLED(0,127,30,0,0);
  }
  else{
    turnOffLED(); // Turn off the Neopixel indicator
  }
}

bool GTernal::isFastCharging(){
  return _flagFastCharging;
}

float GTernal::avgVoltage(float voltNew){
  float sumArray = 0;
  float avgArray;
  int sizeArray = sizeof(_battVoltArray) / sizeof(float);

  _battVoltArray[_battVoltArrayIndx] = voltNew; // Update an element in the array with a new measurement
  for (int i = 0; i < sizeArray; i++){
    sumArray = sumArray + _battVoltArray[i];
  }
  avgArray = sumArray/sizeArray;

  _battVoltArrayIndx++;
  if (_battVoltArrayIndx >= sizeArray){
    _battVoltArrayIndx = 0;
  }
  
  return avgArray;
}

void GTernal::resetVoltageArray(){
  int sizeArray = sizeof(_battVoltArray) / sizeof(float);
  for (int i = 0; i < sizeArray; i++){
    _battVoltArray[i] = 0;
  }
  _voltUpdateTime = millis();
}

void GTernal::monitorBattVolt(){
  if (millis() - _voltUpdateTime >= 1000){ // 1 second interval
    float avgBattVolt = avgVoltage(ina260.readBusVoltage());
    _voltUpdateTime = millis();
    Serial.println(avgBattVolt); // For debugging
    
    if (avgBattVolt > _battVoltThreshold){ // Check if the 30 second average battery voltage is higher than the threshold voltage (mV)
      setFastChargingFlag(0); // Clear the fast charging flag

      // Wake up the Raspberry Pi
      digitalWrite(_rpiEnable, HIGH);
      delay(100);
      digitalWrite(_rpiEnable, LOW);
    }
  }
}

float GTernal::filteredMeasurement(int nSamples, char data[]){
  float sumArray = 0;

  if (strcmp(data, "voltage") == 0){
    for (int i = 0; i < nSamples; i++){
      sumArray = sumArray + ina260.readBusVoltage();
    }
  }
  else if (strcmp(data, "current") == 0){
    for (int i = 0; i < nSamples; i++){
      sumArray = sumArray + ina260.readCurrent();
    }
  }
  else if (strcmp(data, "power") == 0){
    for (int i = 0; i < nSamples; i++){
      sumArray = sumArray + ina260.readPower();
    }
  }
  
  float avgValue = sumArray/nSamples;

  return avgValue;
}
