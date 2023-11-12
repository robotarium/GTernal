#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT.h>
#include "Watchdog_t4.h"

#include <GTernal.h>

GTernal myRobot;
WDT_T4<WDT1> wdt;
int garbage[2];

void setup() {
  Serial.begin(500000);
  Serial3.begin(500000);
  myRobot.SETUP();
  myRobot.rainbow(10);
  myRobot.turnOffLED();

  WDT_timings_t config;
  config.trigger = 1; // Watchdog timer trigger in seconds
  config.timeout = 2; // Watchodog timer timeout in seconds
  wdt.begin(config);
}

void loop() {

  static uint32_t wdt_time = millis();

  if (myRobot.isFastCharging()){
    myRobot.monitorBattVolt();
  }
 else{
   myRobot.jsonSerialRead();
   myRobot.communicationCheck(500);
   myRobot.followCommands();
  }

  if ( millis() - wdt_time > 1100 ) {
    wdt_time = millis();
    wdt.feed();
  }

// For debugging purposes
//  myRobot.getEncoderCounts(garbage);
//   myRobot.PIDMotorControl(1.0, 1.0);
//   myRobot.moveR(100);
//   myRobot.moveL(100);
//   delay(3000);
//   myRobot.noMotion();
//   delay(3000);
//   Serial.println(myRobot.checkBattVoltage());
//   Serial.println(myRobot.checkCharging());
//  Serial.println(ina260.readBusVoltage());
//   Serial.println(myRobot.filteredMeasurement(10,"voltage"));
//   Serial.println(myRobot.filteredMeasurement(10,"current"));
//   Serial.println(myRobot.filteredMeasurement(10,"power"));
}
