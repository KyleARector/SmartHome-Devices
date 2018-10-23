#include <SoftwareSerial.h>

// Declare software serial connection on pins 2 and 3
SoftwareSerial SerialComm(3,2);

const int meter_pin = 0;
const int relay_pin = 7;

const int moisture_threshold = 600;

void setup()
{
  // Start serial connection
  Serial.begin(9600);
  SerialComm.begin(9600);

  pinMode(relay_pin, OUTPUT);

  Serial.println("Starting...");
}

void loop()
{
  /* Automatic Command Section */
  // Read the meter
  int val = analogRead(A0);

  // Evalutate level of moisture
  // Water plants if necessary
  if (val >= moisture_threshold) 
    water_plants_auto();
  /* Automatic Command Section */

  /* Manual Command Section */
  // If serial is available from Node
  if (SerialComm.available() > 0){
    // Read the command
    String command = SerialComm.readString();
    // Water plants
    water_plants_manual();
  }
  /* Manual Command Section */
}

void water_plants_auto()
{
  SerialComm.println("ON");
  // Read the meter
  int val = analogRead(A0);
  // Water in 15 second intervals until moisture threshold is met
  while (val >= moisture_threshold)
  {
    water_plants(15);
    // Check val
    val = analogRead(A0);
  }
  SerialComm.println("OFF");
}

void water_plants_manual()
{
  SerialComm.println("ON");
  water_plants(30);
  SerialComm.println("OFF");
}

void water_plants(int interval_secs)
{
  digitalWrite(relay_pin, HIGH);
  delay(interval_secs * 1000);
  digitalWrite(relay_pin, LOW);
  delay(2000);
}

