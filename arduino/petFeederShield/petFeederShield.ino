#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// Declare second serial connection on pins 4 and 5
SoftwareSerial SerialComm(4,5);

// WiFi Connection Parameters
const char* wifi_ssid = "ssid";
const char* wifi_password = "password";

// MQTT Connection Parameters
const char* mqtt_server = "1.2.3.4";
const char* mqtt_user = "user" ;
const char* mqtt_password = "password";
const int mqtt_port = 1883;

// Sensor Name
const char* sensor_name = "Pet Feeder 1";

// Set Up Topics
const char* node_state = "pet_feeder1";
const char* node_state_set = "pet_feeder1/set";

// Set default state
String state = "OFF";

// Setup time keeping
unsigned long previous_time = 0;
const int rpt_interval_ms = 30000;

// Initialize clients
WiFiClient base_client;
PubSubClient client(base_client);

void pubState()
{
  client.publish(node_state, state.c_str(), true);
}

// Receive the command on the subscribed topic
void recv_command(char* topic, byte* payload, unsigned int length) {
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
    SerialComm.println(command);
    pubState();
  } 
}

// Connect to the MQTT server and subscribe to required topics
void MQTT_connect() {
  while (!client.connected()) {
    if (client.connect(sensor_name, mqtt_user, mqtt_password)) {
      Serial.println("MQTT connected");
      client.subscribe(node_state_set);
    }
    else {
      delay(5000);
    }
  }
  pubState();
}

void setup() {
  // Start USB and software serial connection for debug
  Serial.begin(115200);
  SerialComm.begin(9600);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  // Waiot for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Set up MQTT connection
  client.setServer(mqtt_server, mqtt_port);
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

  // If serial is available from Uno
  if (SerialComm.available() > 0){
    // Read the state
    String readState = SerialComm.readString();
    state = (readState.indexOf("ON") == -1) ? "OFF" : "ON";
  }

  unsigned long current_time = millis();
  if ((unsigned long)(current_time - previous_time) >= rpt_interval_ms)
  {
    pubState();
    previous_time = current_time;
  }
}
