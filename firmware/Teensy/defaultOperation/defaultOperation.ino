#include "Watchdog_t4.h"
#include <GTernal.h>

GTernal myRobot;
WDT_T4<WDT1> wdt;

void setup() {
  // Initialize the USB and UART serial communication
  Serial.begin(500000); // USB Serial for debugging

  // Setup the robot
  myRobot.SETUP();
  myRobot.rainbow(10); // Welcome rainbow light
  myRobot.turnOffLED();

  // Setup the watchdog timer
  WDT_timings_t config;
  config.trigger = 1; // Watchdog timer trigger in seconds
  config.timeout = 2; // Watchodog timer timeout in seconds
  wdt.begin(config);

  Serial3.begin(500000); // UART Serial for Teensy-RPi communication

  // Start the interrupt service routine (ISR) for PID motor control
//  myRobot.beginISR();
}

void loop() {
  // Update the watchdog timer
  static uint32_t wdt_time = millis();

  // Check if the robot is in fast charging mode
  if (myRobot.isFastCharging()){
    myRobot.monitorBattVolt(); // Monitor the battery voltage
  }
  // If not, operate in normal mode
  else{
    myRobot.jsonSerialRead(); // Read JSON messages from the RPi
    myRobot.communicationCheck(500); // Stop the motors if no message is received for 500ms
    myRobot.followCommands();
  }

  // Feed the watchdog timer
  if ( millis() - wdt_time > 1100 ) {
    wdt_time = millis();
    wdt.feed();
  }
}
