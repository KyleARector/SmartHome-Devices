#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi Connection Parameters
const char* wifi_ssid = "ssid";
const char* wifi_password = "password";

// MQTT Connection Parameters
const char* mqtt_server = "1.2.3.4";
const char* mqtt_user = "user" ;
const char* mqtt_password = "password";
const int mqtt_port = 1883;

// Sensor Name
const char* sensor_name = "TV";

// Set Up Topics
const char* node_state = "tv";
const char* node_state_set = "tv/set";

// Set default state
String state = "OFF";

// Setup time keeping
unsigned long previous_time = 0;
const int rpt_interval_ms = 30000;
 
// Declare the pin for IR LED
int pin =  4;

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
    state = command_str;
    tvOnOff();
  }
  pubState();
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

// Pulse the IR emitter
// Parameter is length of pulse in microseconds
void pulseIR(long microsecs) {
  cli();
  while (microsecs > 0) {
   digitalWrite(pin, HIGH);
   delayMicroseconds(10);
   digitalWrite(pin, LOW);
   delayMicroseconds(10);
   microsecs -= 26;
  }
  sei();
}

// Send on/off command to Vizio TV
void tvOnOff() {
  pulseIR(9220);
  delayMicroseconds(4660);
  pulseIR(580);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(600);
  pulseIR(540);
  delayMicroseconds(1740);
  pulseIR(580);
  delayMicroseconds(600);
  pulseIR(540);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(600);
  pulseIR(560);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(600);
  pulseIR(540);
  delayMicroseconds(1740);
  pulseIR(580);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(1760);
  pulseIR(560);
  delayMicroseconds(1760);
  pulseIR(560);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(1740);
  pulseIR(580);
  delayMicroseconds(620);
  pulseIR(520);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(640);
  pulseIR(520);
  delayMicroseconds(600);
  pulseIR(560);
  delayMicroseconds(600);
  pulseIR(560);
  delayMicroseconds(620);
  pulseIR(520);
  delayMicroseconds(1760);
  pulseIR(560);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(1760);
  pulseIR(560);
  delayMicroseconds(620);
  pulseIR(540);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(1740);
  pulseIR(580);
  delayMicroseconds(1740);
  pulseIR(560);
  delayMicroseconds(1760);
  pulseIR(560);
  delayMicroseconds(41300);
  pulseIR(9240);
  delayMicroseconds(2360);
  pulseIR(560);
  delayMicroseconds(33680);
  pulseIR(9240);
  delayMicroseconds(2340);
  pulseIR(560);
  delayMicroseconds(33660);
  pulseIR(9240);
  delayMicroseconds(2340);
  pulseIR(560);
  delayMicroseconds(33660);
  pulseIR(9220);
  delayMicroseconds(2360);
  pulseIR(560);
}

void setup()
{
  // Set the LED pin mode
  pinMode(pin, OUTPUT);

  // Start USB serial connection for debug
  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  // Wait for connection
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

  unsigned long current_time = millis();
  if ((unsigned long)(current_time - previous_time) >= rpt_interval_ms)
  {
    pubState();
    previous_time = current_time;
  }
}
