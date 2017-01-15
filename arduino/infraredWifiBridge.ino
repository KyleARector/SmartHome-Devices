#include <ESP8266WiFi.h>

// WiFi Connection Parameters
const char* ssid = "wifi";
const char* password = "password";

// Declare the pin for IR LED
int pin =  4;

// Set default state
// In power failure, switches/IR blasters would be turned off
String state = "OFF";

// Create an instance of the server
WiFiServer server(80);

// Forward declaration of methods
void pulseIR(long microsecs);
void tvOnOff();
String genResponse(String val);

void setup()
{
  // Set the LED pin mode
  pinMode(pin, OUTPUT);

  // Start Serial
  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Start WiFi connection
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  // Do not broadcast AP
  WiFi.mode(WIFI_STA);
}

void loop()
{
  // Check for client
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait for client data
  while(!client.available()){
    delay(1);
  }

  // Read the request and verb
  String req = client.readString();
  String verb = req.substring(0, req.indexOf(' '));
  client.flush();

  // Exit process if not valid endpt
  if (req.indexOf("/state") == -1) {
    client.stop();
    return;
  }
  client.flush();

  String respVal = "";
  // Handle GET request
  if (verb == "GET") {
    respVal = state;
  }
  // Handle POST request
  else if (verb == "POST") {
    if (req.indexOf("ON") >=0 && state != "ON") {
      state = "ON";
      respVal = state;
      tvOnOff();
    }
    else if (req.indexOf("OFF") >=0 && state != "OFF") {
      state = "OFF";
      respVal = state;
      tvOnOff();
    }
  }

  // Send the response
  client.print(genResponse(respVal));
  delay(1);
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

// Generate HTTP response
// Parameter is on/off value to be inserted into response
String genResponse(String val) {
  String resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
  resp += "{\"state\": \"";
  resp += val;
  resp += "\"}";
  resp += "\n";
  return resp;
}

// Work in progress to dynamically send pulses from array
// Array automatically generated from remote capture sketch
/*void sendSignal(int signalArray[])
{
  for(int x = 0; x < sizeof(signalArray); x++)
  {
    if((x % 2) == 0)
    {
      pulseIR(signalArray[x] * 10);
      Serial.println(signalArray[x] * 10);
    }
    else
    {
      delayMicroseconds(signalArray[x] * 10);
      Serial.println(signalArray[x] * 10);
    }
  }
}*/
