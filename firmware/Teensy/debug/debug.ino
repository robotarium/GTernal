#include <GTernal.h>

GTernal myRobot;
int encoderCount[2];
float wheelSpeed[2];
StaticJsonBuffer<2048> _jsonBufferOut;

void setup() {
  Serial.begin(500000);
  Serial3.begin(500000);
  myRobot.SETUP();
  myRobot.beginISR();
  Serial.println("Setup Finished");
}

void loop() {
  // Motor
  myRobot.setVelocity(0.032, 0.0);
  myRobot.getWheelSpeeds(wheelSpeed);
  Serial.print("Left:");
  Serial.print(wheelSpeed[0]);
  Serial.print(" ");
  Serial.print("Right:");
  Serial.println(wheelSpeed[1]);

  // ToF sensor
  JsonObject& jsonOut = _jsonBufferOut.createObject();
  JsonArray& distanceArray = jsonOut.createNestedArray("distances");
  myRobot.measureDistances(distanceArray);
  jsonOut.printTo(Serial);
  Serial.println("");
  _jsonBufferOut.clear();
  delay(10);

  // Encoder
  //  myRobot.getEncoderCounts(encoderCount);

  // Charging status
  //   Serial.println(myRobot.checkBattVoltage());
  //   Serial.println(myRobot.checkCharging());
  //  Serial.println(ina260.readBusVoltage());
  //   Serial.println(myRobot.filteredMeasurement(10,"voltage"));
  //   Serial.println(myRobot.filteredMeasurement(10,"current"));
  //   Serial.println(myRobot.filteredMeasurement(10,"power"));
}
