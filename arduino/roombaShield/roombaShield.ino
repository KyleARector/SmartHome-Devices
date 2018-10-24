#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Declare second serial connection on pins 4 and 5
SoftwareSerial SerialComm(4,5);

// WiFi Connection Parameters
const char* wifi_ssid = "ssid";
const char* wifi_password = "password";

// MQTT Connection Parameters
const char* mqtt_server = "1.2.3.4";
const char* mqtt_user = "username" ;
const char* mqtt_password = "password";
const int mqtt_port = 1883;

// Sensor Name
const char* sensor_name = "Roomba";

// Set Up Topics
const char* node_state = "vacuum/state";
const char* node_state_set = "vacuum/command";

// Setup time keeping
unsigned long previous_time = 0;
const int rpt_interval_ms = 15000;

// Initialize clients
WiFiClient base_client;
PubSubClient client(base_client);

bool readDataFromSerial(uint8_t* dest, uint8_t len)
{
  while (len-- > 0)
  {
    unsigned long startTime = millis();
    while (!SerialComm.available())
    {
      if (millis() > startTime + 200) {
        return false; // Read time out
      }
    }
    *dest++ = SerialComm.read();
  }
  return true;
}

void readSensorsAndPubState() {
  // Clear Buffer
  while (SerialComm.available()) {
    SerialComm.read();
  }

  uint8_t sensorIds[] = {
    19, // Distance Travelled
    21, // Charging State
    22, // Voltage
    23, // Current
    25, // Battery Charge
    26  // Battery Capacity
  };

  // Array to store results
  uint8_t sensorVals[11];

  // Send mode request
  SerialComm.write(149);
  // Request specific sensor packets
  SerialComm.write(6);
  SerialComm.write(sensorIds, 6);

  if (!readDataFromSerial(sensorVals, 11))
    return;
  
  int16_t distance = sensorVals[0] * 256 + sensorVals[1];
  uint8_t chargingState = sensorVals[2];
  uint16_t voltage = sensorVals[3] * 256 + sensorVals[4];
  int16_t current = sensorVals[5] * 256 + sensorVals[6];
  uint16_t charge = sensorVals[7] * 256 + sensorVals[8];
  uint16_t capacity = sensorVals[9] * 256 + sensorVals[10];

  bool cleaning = false;
  bool docked = false;

  if (current < -300) {
    cleaning = true;
  } else if (current > -50) {
    docked = true;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["battery_level"] = (charge * 100)/capacity;
  root["cleaning"] = cleaning;
  root["docked"] = docked;
  root["charging"] = chargingState == 1
  || chargingState == 2
  || chargingState == 3;
  root["voltage"] = voltage;
  root["current"] = current;
  root["charge"] = charge;
  String jsonStr;
  root.printTo(jsonStr);
  client.publish(node_state, jsonStr.c_str());
}

// Receive the command on the subscribed topic
void recvCommand(char* topic, byte* payload, unsigned int length) {
  char command[length + 1];
  for (int i = 0; i < length; i++) {
    command[i] = (char)payload[i];
  }
  command[length] = '\0';
  // Only update the state and issue command if different state requested
  String command_str = command;
  // Send the new command to the Roomba
  if (command_str == "turn_on")
    start_cleaning();
  else
    seek_dock();
  readSensorsAndPubState();
}

void start_cleaning() {
  Serial.println("Starting clean...");
  SerialComm.write(128);
  delay(50);
  SerialComm.write(131);
  delay(50);
  SerialComm.write(135);
  delay(50);
}

void seek_dock() {
  Serial.println("Stopping...");
  SerialComm.write(128);
  delay(50);
  SerialComm.write(131);
  delay(50);
  SerialComm.write(143);
  delay(50);
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
  readSensorsAndPubState();
}

void setup() {
  // Start USB and software serial connection for debug
  Serial.begin(115200);
  SerialComm.begin(115200);

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
  client.setCallback(recvCommand);
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

  // Read sensor and send out reports at regular intervals
  unsigned long current_time = millis();
  if ((unsigned long)(current_time - previous_time) >= rpt_interval_ms)
  {
    readSensorsAndPubState();
    previous_time = current_time;
  }
}
