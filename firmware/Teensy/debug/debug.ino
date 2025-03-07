#include <GTernal.h>

GTernal myRobot;
int encoderCount[2];
float wheelSpeed[2];
float interruptTime[1];
StaticJsonBuffer<2048> _jsonBufferOut;

void setup() {
  Serial.begin(500000);
  Serial3.begin(500000);
  myRobot.SETUP();
  myRobot.beginISR();
  Serial.println("Setup Finished");

  while(1){
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 'a'){
        break;
      }
    }
  }
}

void loop() {
  // Motor
  myRobot.setVelocity(0.016, 0.0);
  myRobot.getWheelSpeeds(wheelSpeed);
//  Serial.print("Left:");
  Serial.println(wheelSpeed[0]);
//  Serial.print(" ");
//  Serial.print("Right:");
//  Serial.println(wheelSpeed[1]);
  delay(1);

//  myRobot.getISRTime(interruptTime);
//  Serial.println(interruptTime[0]);
//  delay(1);

//  // ToF sensor
//  JsonObject& jsonOut = _jsonBufferOut.createObject();
//  JsonArray& distanceArray = jsonOut.createNestedArray("distances");
//  myRobot.measureDistances(distanceArray);
//  jsonOut.printTo(Serial);
//  Serial.println("");
//  _jsonBufferOut.clear();
//  delay(10);

  // Encoder
//    myRobot.getEncoderCounts(encoderCount);
//    Serial.println(encoderCount[0]);
//    delay(1);
//    Serial.println(encoderCount[1]);

  // Charging status
  //   Serial.println(myRobot.checkBattVoltage());
  //   Serial.println(myRobot.checkCharging());
  //  Serial.println(ina260.readBusVoltage());
  //   Serial.println(myRobot.filteredMeasurement(10,"voltage"));
  //   Serial.println(myRobot.filteredMeasurement(10,"current"));
  //   Serial.println(myRobot.filteredMeasurement(10,"power"));
}
