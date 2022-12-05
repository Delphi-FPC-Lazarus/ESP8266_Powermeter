/*
  Arduino IDE Project for ESP8266 Board

  This is an simple project for ESP8266 (NodeMCU 1.1 ESP-12E) Module. 
  Webserver providing a few Endpoints for analog meassurement from adc (TC-9520256 / TA12-200)
  
  by Peter Lorenz
  support@peter-ebe.de
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "WLANZUGANGSDATEN.h"

#define ANALOG_IN 0
#define CALPOWER 358
#define CALVALUE 288
#define CALVALUEOFFSET 0

const char* ssid = STASSID;
const char* password = STAPSK;

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 240);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);   //optional
IPAddress secondaryDNS(192, 168, 1, 1); //optional

ESP8266WebServer server(80);

void handleRoot() {
  Serial.println("handleRoot()");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  String message = "esp8266 powermeter online\r\n";

  message += "Connected to ";
  message += String(ssid);
  message += "\r\n";
  message += "IP address: ";
  message += String(WiFi.localIP().toString());
  message += "\r\n";

  server.send(200, "text/plain", message);

  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void handleSampleValues() {
  Serial.println("handleSampleValues()");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  int values[500];
  for (int i = 0; i < ( sizeof(values) / sizeof(values[0]) ) - 1 ; i++) 
  {
    values[i] = analogRead(ANALOG_IN); // relativ träge, 0,1ms auf dem esp8266
    //delay() < 1ms compiliert, tut aber nichts                                              
  } 
  String message = "";
  for (int i = 0; i < ( sizeof(values) / sizeof(values[0]) ) - 1 ; i++) 
  {
    message += values[i];
    message += ";";
  }
  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

int getMaxValue() {
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  int values[500];
  for (int i = 0; i < ( sizeof(values) / sizeof(values[0]) ) - 1 ; i++) 
  {
    values[i] = analogRead(ANALOG_IN); // relativ träge, 0,1ms auf dem esp8266
    //delay() < 1ms compiliert, tut aber nichts                                               
  }
  int maxValue = 0; 
  for (int i = 0; i < ( sizeof(values) / sizeof(values[0]) ) - 1 ; i++) 
  {
    if (values[i] > maxValue) {
      maxValue = values[i];
    }
  }
  return maxValue;
}

void handleMaxValue() {
  Serial.println("handleMaxValue()");

  String message = String(getMaxValue());
  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void handleMeasurePower() {
  Serial.println("handleMeasurePower");

  int value = getMaxValue();
  Serial.println("Value:");
  Serial.println(value);
  float power = float(CALPOWER)/float(CALVALUE) * value-CALVALUEOFFSET;
  Serial.println("Power");
  Serial.println(power);

  String message = String(power);
  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void handleNotFound() {
  Serial.println("handleNotFound()");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  // Wifi
  Serial.begin(115200);
    
  WiFi.mode(WIFI_STA);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // MDNS
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // Server
  server.on("/", handleRoot);
  server.on("/sampleValues", handleSampleValues);
  server.on("/maxValue", handleMaxValue);

  server.on("/measurePower", handleMeasurePower);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
