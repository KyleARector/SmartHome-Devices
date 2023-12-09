#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "config.h"

// Set default state
String state = "0";
int debounce = 0;

const int button1Pin = 14;
const int button2Pin = 4;
const int button3Pin = 5;
const int button4Pin = 15;
const int button5Pin = 13;
const int button6Pin = 12;

// Initialize clients
WiFiClient base_client;
PubSubClient client(base_client);

void ICACHE_RAM_ATTR buttonPressed(String button)
{
  if (debounce == 0)
  {
    debounce = 10;
    state = button;
    pubState();
  }
}

void ICACHE_RAM_ATTR button1Interrupt()
{
  buttonPressed("1");
}

void ICACHE_RAM_ATTR button2Interrupt()
{
  buttonPressed("2");
}

void ICACHE_RAM_ATTR button3Interrupt()
{
  buttonPressed("3");
}

void ICACHE_RAM_ATTR button4Interrupt()
{
  buttonPressed("4");
}

void ICACHE_RAM_ATTR button5Interrupt()
{
  buttonPressed("5");
}

void ICACHE_RAM_ATTR button6Interrupt()
{
  buttonPressed("6");
}

void pubState()
{
  client.publish(NODE_STATE_SET_TOPIC, state.c_str(), true);
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

  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);
  pinMode(button5Pin, INPUT_PULLUP);
  pinMode(button6Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1Pin), button1Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(button2Pin), button2Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(button3Pin), button3Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(button4Pin), button4Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(button5Pin), button5Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(button6Pin), button6Interrupt, RISING);
  
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

  ArduinoOTA.handle();

  if (debounce > 0)
  {
    delay(100);
    debounce--;
  }
}
