#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Initialize motor shield
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Use M4 on motor shield
Adafruit_DCMotor *motor = AFMS.getMotor(4);

// Declare software serial connection on pins 2 and 3
SoftwareSerial SerialComm(4,5);

// Declare dispense function
void dispense();

void setup()
{
  // Start serial connection
  Serial.begin(9600);
  SerialComm.begin(9600);

  Serial.println("Starting...");
  // Start the motor shield
  AFMS.begin();
  // Set motor default speed to highest possible
  motor->setSpeed(255); 
}

void loop() {
  // If serial is available from Node
  if (SerialComm.available() > 0){
    // Read the command
    String command = SerialComm.readString();
    // Dispense food
    dispense();
  }
}

void dispense()
{
  SerialComm.println("ON");
  int dispenseInterval = 10000;
  motor->run(FORWARD);
  delay(dispenseInterval);
  motor->run(RELEASE);
  SerialComm.println("OFF");
}

