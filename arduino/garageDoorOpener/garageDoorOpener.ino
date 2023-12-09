#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "config.h"

// Set default state
String state = "OFF";

// Setup time keeping
unsigned long previous_time = 0;
const int rpt_interval_ms = 1000;

const int sensePin = 4;
const int relayPin = 5;

// Initialize clients
WiFiClient base_client;
PubSubClient client(base_client);

void switchRelay() 
{
  digitalWrite(relayPin, HIGH);
  delay(250);
  digitalWrite(relayPin, LOW);
}

void pubState()
{
  client.publish(NODE_STATE_TOPIC, state.c_str(), true);
}

void readState()
{
  state = digitalRead(sensePin) == HIGH ? "ON" : "OFF";
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
    // Switch the relay on and off
    switchRelay();
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

  // Set up pins for relay and sensor
  pinMode(sensePin, INPUT);
  digitalWrite(relayPin, LOW);
  pinMode(relayPin, OUTPUT);

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
    readState();
    pubState();
    previous_time = current_time;
  }

  ArduinoOTA.handle();
}
