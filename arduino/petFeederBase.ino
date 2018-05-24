#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Initialize motor shield
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Use M4 on motor shield
Adafruit_DCMotor *motor = AFMS.getMotor(4);

// Declare software serial connection on pins 2 and 3
SoftwareSerial SerialComm(3,2);

// Declare dispense function
void dispense(int cups);

void setup()
{
  // Start serial connection
  Serial.begin(9600);
  SerialComm.begin(9600);
  // Start the motor shield
  AFMS.begin();
  // Set motor default speed to highest possible
  motor->setSpeed(255); 
}

void loop()
{
  // If serial is available from Node
  if (SerialComm.available() > 0){
    // Read the character and cast as int
    int cups = atoi(SerialComm.read());
    // Dispense food
    dispense(cups);
  }
}

void dispense(int cups)
{
  int dispenseInterval = 5000 * cups;
  motor->run(FORWARD);
  delay(dispenseInterval);
  motor->run(RELEASE);
}

