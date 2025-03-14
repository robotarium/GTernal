#ifndef GTernal_h
#define GTernal_h

#include "Arduino.h"
#include "Encoder.h"
#include "ArduinoJson.h"
#include "Adafruit_NeoPixel.h"
#include <Adafruit_INA260.h>
#include <Adafruit_BNO055.h>
#include <vl53l4cd_class.h>
#include <Wire.h>
#include <string>
#include <array>

using namespace std;

class GTernal
{
  public:
    GTernal();
    static GTernal* _instance; //Singleton Instance
    void beginISR();

    ///////////////////////////////////////////////////////////
    //Functions
    ///////////////////////////////////////////////////////////

    void SETUP(); //Setup Routine

    float convertUnicycleToRightMotor(float vel, float w); // Unicycle Model Conversions
    float convertUnicycleToLeftMotor(float vel, float w); // Unicycle Model Conversions

    void setSingleLED(int pos, int r, int g, int b, int w); // Set a single Neo Pixel's color.
    void setAllLED(int r, int g, int b, int w); // Set both Neo Pixels to the same color.
    void turnOffLED(); // Turn off Neo Pixels.
    void rainbow(uint8_t wait); // Display a rainbow effect on the LED (CAUTION: INCLUDES A DELAY)

    void getEncoderCounts(int encoderData[]); // Read and store the encoders tick counts in the argument array [Left, Right]
    void getWheelSpeeds(float wheelSpeeds[]); // Read and store the wheel speeds in the argument array [Left, Right]
    void updateWheelSpeeds(); // Update the wheel speeds
    void getISRTime(float interruptTime[]); // Get the time elapsed in the ISR
    bool checkCharging(); // Returns true if the battery is currently charging
    float checkBattVoltage(); // Reads and returns the current battery voltage

    void enableMuxPort(uint8_t port); // Enable the desired port on the Mux
    void measureDistances(JsonArray& array); // Read and store the sensor distances in the json array passed into the function
    void getOrientation(JsonArray& oriArray); // Read and store the orientation data in the json array passed into the function
    
    void encoderPositionUpdate(float timeStep); //Encoders used for state estimates.
    void setGlobalPosition(float x, float y, float a); // Set the global state of the robot.
    void getGlobalPosition(float state[]); //Get the global state of the robot. [x,y,theta]
    
    void moveL(int motorSpeed); //Rotate Left Motor Forward at Speed motorSpeed (-255 to 255)
    void moveR(int motorSpeed); //Rotate Right Motor Forward at Speed motorSpeed (-255 to 255)

    void brake();//Short the Motors to Brake (No Passive Rotation)
    void noMotion();//Supply no current to motors.

    void jsonSerialRead();//Reads JSON messages from the RPi

    void followCommands();//Executes commands.

    void communicationCheck(float communicationTimeout);//Will stop the motors if no communication is received.

    void PIDMotorControl(float desLVelInput, float desRVelInput); //PID Controller to Keep Wheels Rotating at Proper Speed
    void openLoopControl(float desLVelInput, float desRVelInput); //Open Loop Control to Keep Wheels Rotating at Proper Speed

    void setFastChargingFlag(bool flag); // Fast charging flag
    bool isFastCharging(); // Returns if the robot is in fast charging mode or not
    float avgVoltage(float voltNew); // Returns 30 seconds average battery voltage
    void resetVoltageArray(); // Resets the voltage array for 30 seconds average battery voltage
    void monitorBattVolt();
    float filteredMeasurement(int nSamples, char data[]);

    void setVelocity(float v, float w); // Set the desired velocity and angular velocity of the robot

    
  private:
    static IntervalTimer timerISR; //Timer for PID Motor Control
    static void isrHandler(); //Interrupt Handler for PID Motor Control
    float _ISRElapsedTime; //Elapsed Time in ISR
    int _motorSpeedR_old = 0; //Old Right Motor Speed
    int _motorSpeedL_old = 0; //Old Left Motor Speed

    ///////////////////////////////////////////////////////////
    //Constants
    ///////////////////////////////////////////////////////////
    static constexpr float _pi = 3.1415926; // Pi...the number...DUH!!
    static constexpr int _encoderCountsPerRotation = 28; // Encoder counts per single shaft rotation.
    static constexpr float _motorGearRatio = 100; // The gearing ratio of the drive motor being used.
    static constexpr float _wheelDiameter = 0.032; // Wheel Diameter in cm.
    static constexpr float _axelLength = 0.11; // Axel length in cm.
    static constexpr float _battVoltThreshold = 4000.0; // Threshold battery voltage (mV) for turning turning the RPi on when in fast charging mode

    ///////////////////////////////////////////////////////////
    //Pin Numbers Here
    ///////////////////////////////////////////////////////////

    //Motor control pins

    static constexpr uint8_t _PWMR = 4;// D4 // Right Motor PWM Control
    static constexpr uint8_t _PWML = 23;// D23 // Left Motor PWM Control

    static constexpr uint8_t _STBY = 6;// D6 // Standby pin to shut off motor board
    static constexpr uint8_t _motEN = 5;// D5 // Motor enable pin

    static constexpr uint8_t _RMotor1 = 0;// D0 // Right Motor Direction 1
    static constexpr uint8_t _RMotor2 = 1;// D1 // Right Motor Direction 2

    static constexpr uint8_t _LMotor1 = 7;// D7 // Left Motor Direction 1
    static constexpr uint8_t _LMotor2 = 8;// D8 // Left Motor Direction 2

    //Encoder feedback pins

    static constexpr uint8_t _interruptL1 = 20;// D20 // Left motor encoder interrupt 1
    static constexpr uint8_t _interruptR1 = 2;// D2 // Right motor encoder interrupt 1

    static constexpr uint8_t _interruptL2 = 21;// D21 // Left motor encoder interrupt 2
    static constexpr uint8_t _interruptR2 = 3;// D3 // Right motor encoder interrupt 2

    //Battery feedback pins

    static constexpr uint8_t _battVoltage = A8;// A8 // Analog battery voltage measurement
    static constexpr uint8_t _battChargingCheck = 13;// D13 // Digital battery charging check

    //RPi enable pin

    static constexpr uint8_t _rpiEnable = 9;// D13 // Digital battery charging check

    ///////////////////////////////////////////////////////////
    //Functions
    ///////////////////////////////////////////////////////////

    void _readEncoders(); //Reads the current encoder counts and stores them in _encoderCountL, _encoderCountR;

    ///////////////////////////////////////////////////////////
    //JSON Messaging Objects
    ///////////////////////////////////////////////////////////

    StaticJsonBuffer<2048> _jsonBufferIn; //If JSON message fails unexplainably, increase buffer size.
    StaticJsonBuffer<2048> _jsonBufferOut;

    //Storage for what the request from the RPi should be.
    // 0: Read
    // 1: Write
    int _method = -1;
    float _communicationTimeout; // Timer since last communication

    ///////////////////////////////////////////////////////////
    //ToF Distance Sensor
    ///////////////////////////////////////////////////////////
    const int _NUMBER_OF_SENSORS = 6;  // Number of VL53L4CD sensors on GTernal. N_sensors - 1 for indexing purposes.
    
    ///////////////////////////////////////////////////////////
    //IR Sensor Distance Variables
    ///////////////////////////////////////////////////////////

    float _RDistance; //Right Facing IR Sensor Distance Storage.
    float _FDistance; //Forward Facing IR Sensor Distance Storage.
    float _LDistance; //Left Facing IR Sensor Distance Storage.
    float _LFDistance; //Left Forward Facing IR Sensor Distance Storage.
    float _RFDistance; //Right Forward Facing IR Sensor Distance Storage.
    float _LBDistance; //Left Backwards Facing IR Sensor Distance Storage.
    float _RBDistance; //Right Backwards Facing IR Sensor Distance Storage.

    ///////////////////////////////////////////////////////////
    //Encoder Counter Variables
    ///////////////////////////////////////////////////////////

    int _encoderCountL; // Left wheel encoder count.
    int _encoderCountR; // Right wheel encoder count.

    volatile float _wheelSpeedL; // Left wheel speed in rad/s.
    volatile float _wheelSpeedR; // Right wheel speed in rad/s.


    ///////////////////////////////////////////////////////////
    //IR Distance Conversion Constants
    ///////////////////////////////////////////////////////////

    //TO DO! CALIBRATION

    //Constants from distance conversion see getDistance()

    static constexpr double _a = 15.68;// 15.68
    static constexpr double _b = -0.03907;// -0.03907

    ///////////////////////////////////////////////////////////
    //Motor PID Control Constants
    ///////////////////////////////////////////////////////////

    float _PIDMotorsTimeStart;
    float _encoderTimeStart;
    float _PIDTimeStep = 0.01; // 0.01s = 10 ms (100 Hz)
    bool _driveStart;
    bool _encoderStart;
    volatile float _v;
    volatile float _w;

    static constexpr float _kpMotor = 2.7; //6.0;// = 0.904;
    static constexpr float _kiMotor = 0.9; //3.0;// = 146;
    static constexpr int _kdMotor = 0.95; //0.3;// = 0;
    static constexpr float _motorDigitalK = 1;// = 0.544;

    int _motorL = 0;//Left Motor Speed (Arduino PWM Units, int 0-255)
    int _motorR = 0;//Right Motor Speed (Arduino PWM Units, int 0-255)

    int _oldMotorL;//Old Left Motor Speed (Arduino PWM Units, int 0-255). Used to limit acceleration of motor to prevent voltage sag.
    int _oldMotorR;//Old Right Motor Speed (Arduino PWM Units, int 0-255). Used to limit acceleration of motor to prevent voltage sag.

    int _maxMotorInc = 50;//Maximum PWM increase in a time step (0.01).

    int _oldMotorPIDEncoderCountL;//Old Encoder Count storage for PIDMotorControl Function
    int _oldMotorPIDEncoderCountR;

    float _oldErrorL; //old speed error of motor for PIDMotorControl Function
    float _oldErrorR;

    float _integralL; //last integral error count
    float _integralR;

    // Variables for prefilter in controller
    float _desVelR;
    float _desVelL;

    // Variables to deal with integral windup due to saturation
    bool _satR;
    int _satRVal;
    bool _satL;
    int _satLVal;

    int _speedBufferCount = 0; // Buffer count for wheel speed
    float _wheelSpeedLBuffer[10]; // Buffer for left wheel speed
    float _wheelSpeedRBuffer[10]; // Buffer for right wheel speed

    ///////////////////////////////////////////////////////////
    //Rotational PID Control Constants
    ///////////////////////////////////////////////////////////

    // Rotational Controller Gains!
    static constexpr float _kpAng = 0.932;
    static constexpr float _kiAng = 0;
    static constexpr float _kdAng = 0;
    static constexpr float _rotationalDigitalK = 2.5;

    ///////////////////////////////////////////////////////////
    //Encoder Variables
    ///////////////////////////////////////////////////////////

    Encoder _motorLeft, _motorRight;

    ///////////////////////////////////////////////////////////
    //LED Variables
    ///////////////////////////////////////////////////////////

    Adafruit_NeoPixel _strip;

    ///////////////////////////////////////////////////////////
    //Encoder Position Update Variables
    ///////////////////////////////////////////////////////////

    float _positionUpdateTimeStart;

    int _oldEPUEncoderCountL;//Old Encoder Count storage for encoderPositionUpdate Function
    int _oldEPUEncoderCountR;

    float _botVel; //Robot current linear velocity
    float _botXPos; //Robot current global x position
    float _botYPos; //Robot current global y position
    float _botA; //Robot current global orientation

    float _botA0;//Storage for last heading of robot used in encoderPositionUpdate Fucntion  

    ///////////////////////////////////////////////////////////
    //Fast Charging Mode
    ///////////////////////////////////////////////////////////
    bool _flagFastCharging = 0;
    float _battVoltArray[30];
    int _battVoltArrayIndx = 0;
    unsigned long _voltUpdateTime;

    ///////////////////////////////////////////////////////////
    //Useful Functions
    ///////////////////////////////////////////////////////////

    float _deg2rad(float deg); //Converts degrees to radians.
    float _rad2deg(float rad); //Converts radians to degrees.
    float _wrapToPi(float rad); //Wraps a radian angle to (-pi,pi]
    float _wrapTo2Pi(float rad); //Wrap a radian angle to [0,2*pi)
    uint32_t _wheel(uint8_t wheelPos); //Given a uint8_t will return an RGB color

};

#endif
