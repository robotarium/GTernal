#ifndef GritsbotX3_h
#define GritsbotX3_h

#include "Arduino.h"
#include "Encoder.h"
#include "ArduinoJson.h"
#include "Adafruit_NeoPixel.h"
#include <Adafruit_INA260.h>


class GritsbotX
{
  public:
    GritsbotX();

    ///////////////////////////////////////////////////////////
    //Functions
    ///////////////////////////////////////////////////////////

    void SETUP(); //Setup Routine

    float convertUnicycleToRightMotor(float vel, float w); // Unicycle Model Conversions
    float convertUnicycleToLeftMotor(float vel, float w); // Unicycle Model Conversions

    void setSingleLED(int pos, int r, int g, int b); // Set a single Neo Pixel's color.
    void setAllLED(int r, int g, int b); // Set both Neo Pixels to the same color.
    void turnOffLED(); // Turn off Neo Pixels.
    void rainbow(uint8_t wait); // Display a rainbow effect on the LED (CAUTION: INCLUDES A DELAY)

    void getEncoderCounts(int encoderData[]); // Read and store the encoders tick counts in the argument array [Left, Right]
    bool checkCharging(); // Returns true if the battery is currently charging
    float checkBattVoltage(); // Reads and returns the current battery voltage

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

  private:

    ///////////////////////////////////////////////////////////
    //Constants
    ///////////////////////////////////////////////////////////
    static constexpr float _pi = 3.1415926; // Pi...the number...DUH!!
    static constexpr int _encoderCountsPerRotation = 28; // Encoder counts per single shaft rotation.
    static constexpr float _motorGearRatio = 100.37; // The gearing ratio of the drive motor being used.
    static constexpr float _wheelDiameter = 0.032; // Wheel Diameter in cm.
    static constexpr float _axelLength = 0.105; // Axel length in cm.

    ///////////////////////////////////////////////////////////
    //Pin Numbers Here
    ///////////////////////////////////////////////////////////

    //Motor control pins

    static constexpr uint8_t _PWMR = 4;// D4 // Right Motor PWM Control
    static constexpr uint8_t _PWML = 23;// D23 // Left Motor PWM Control

    static constexpr uint8_t _STBY = 6;// D6 // Standby pin to shut off motor board

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
    bool _driveStart;
    float _v;
    float _w;

    static constexpr float _kpMotor = 1.00;// = 0.904;
    static constexpr float _kiMotor = 0.1;// = 146;
    static constexpr int _kdMotor = 0.00;// = 0;
    static constexpr float _motorDigitalK = 1;// = 0.544;

    int _motorL;//Left Motor Speed (Arduino PWM Units, int 0-255)
    int _motorR;//Right Motor Speed (Arduino PWM Units, int 0-255)

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
    //Useful Functions
    ///////////////////////////////////////////////////////////

    float _deg2rad(float deg); //Converts degrees to radians.
    float _rad2deg(float rad); //Converts radians to degrees.
    float _wrapToPi(float rad); //Wraps a radian angle to (-pi,pi]
    float _wrapTo2Pi(float rad); //Wrap a radian angle to [0,2*pi)
    uint32_t _wheel(byte wheelPos); //Given a byte will return an RGB color

};

#endif
