#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// WiFi Connection Parameters
const char* ssid = "wifi";
const char* password = "password";

// Declare second serial connection on pins 4 and 5
SoftwareSerial SerialComm(4,5);

// Set default state
// In power failure, switches/IR blasters would be turned off
String state = "OFF";

// Create an instance of the server
WiFiServer server(80);

// Forward declaration of methods
void curtainControl(String command);
String genResponse(String val);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);
  SerialComm.begin(9600);

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

void loop() {
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
      curtainControl("O");
    }
    else if (req.indexOf("OFF") >=0 && state != "OFF") {
      state = "OFF";
      respVal = state;
      curtainControl("C");
    }
  }

  // Send the response
  client.print(genResponse(respVal));
  delay(1);
}

// Send the received command to the base Arduino over serial
void curtainControl(String command) {
  Serial.println(command);
  SerialComm.println(command);
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
