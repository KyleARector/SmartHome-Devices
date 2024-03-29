#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "config.h"

// Set default state
String state = "OFF";

// Setup time keeping
unsigned long previous_time = 0;
const int rpt_interval_ms = 30000;
unsigned long previous_state_time = 0;
const int state_timeout_interval_ms = 60000;

const int speedPin = 5;
const int dirPin = 0;

enum speeds {
  STOP = 0,
  SLOW = 300,
  MED = 600,
  FAST = 1000
};

enum direction {
  FORWARD = 0,
  REVERSE = 1
};

// Initialize clients
WiFiClient base_client;
PubSubClient client(base_client);

void runMotor(int seconds) 
{
  delay(50);
  analogWrite(speedPin, FAST);
  delay(seconds * 1000);
  analogWrite(speedPin, STOP);
}

void pubState()
{
  client.publish(NODE_STATE_TOPIC, state.c_str(), true);
}

// Receive the command on the subscribed topic
void recv_command(char* topic, byte* payload, unsigned int length)
{
  char command[length + 1];
  for (int i = 0; i < length; i++) {
    command[i] = (char)payload[i];
  }
  command[length] = '\0';
  // Only update the state and issue command if different state requested
  String command_str = command;
  if (command_str != state)
  {
    state = command;
    // Send the new command to the Arduino Uno
    runMotor(6);
    pubState();
    previous_state_time = millis();
  } 
}

// Connect to the MQTT server and subscribe to required topics
void MQTT_connect()
{
  while (!client.connected()) {
    if (client.connect(SENSOR_NAME, MQTT_USER, MQTT_PASSWD)) {
      Serial.println("MQTT connected");
      client.subscribe(NODE_STATE_SET_TOPIC);
    }
    else {
      delay(5000);
    }
  }
  pubState();
}

void setup()
{
  // Start USB and software serial connection for debug
  Serial.begin(115200);

  // Set up pins for motor controller
  pinMode(dirPin, OUTPUT);
  pinMode(speedPin, OUTPUT);

  // Set initial state of motor pins
  analogWrite(speedPin, STOP);
  digitalWrite(dirPin, FORWARD);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ArduinoOTA.begin();

  // Set up MQTT connection
  client.setServer(MQTT_SERV, MQTT_PORT);
  client.setCallback(recv_command);
  MQTT_connect();
}

void loop()
{
  // Reconnect if server lost
  if (!client.connected()) {
    //ESP.reset();
    MQTT_connect();
  }

  // Run the MQTT client loop
  client.loop();

  unsigned long current_time = millis();
  if ((unsigned long)(current_time - previous_time) >= rpt_interval_ms)
  {
    pubState();
    previous_time = current_time;
  }
  if (((unsigned long)(current_time - previous_state_time >= state_timeout_interval_ms))
      && state != "OFF")
  {
    state = "OFF";
    pubState();
  }

  ArduinoOTA.handle();
}
