/*
  Arduino IDE Project for ESP8266 Board

  This is an simple project for ESP8266 (NodeMCU 1.1 ESP-12E) Module. 
  Webserver providing a few Endpoints for analog meassurement from adc (TC-9520256 / TA12-200)
  for sinusoidal power consumption 
  
  by Peter Lorenz
  support@peter-ebe.de
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "WLANZUGANGSDATEN.h"

#define ANALOG_IN 0
#define CALCURRENTPOWER 358
#define CALCURRENTVALUE 288

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

// --------------------------------------------

// Messfunktionen 
// Notiz: delay() < 1ms compiliert, tut aber nichts                                              
// analogRead() ist träge, 0,1ms auf dem esp8266
// Wechselspannungsmessung bzw. nur positive Halbwelle
// 50hz = 20ms -> 200 Samples je Sinus 
// Um dieses Signal zu messen muss die Anzahl der Samples mindest ein Sinus sein, oder ein gerades vielfaches davon
const int sampleCount = 600;
// nach dem sortieren die höchsten n values wegschmeißen 
const int skipValues = 5;
// und über die n niedrigeren mitteln
const int measureValues = 10;

// Für die Messung von dynamischen Lasten muss über einen längeren Zeitraum gemittelt werden
const int avgCount = 100;
bool avgActive = false;
int avgValue = 0;
int values[sampleCount];
// iterieren über array über samplecount oder (sizeof(values) / sizeof(values[0]))

void exValue(int i, int j) {
 int temp = values[i];
 values[i] = values[j];
 values[j] = temp;
}
void qSort(int loIdx, int highIdx) {
 int i = loIdx;
 int j = highIdx;
 int pivot = values[loIdx + (highIdx - loIdx) / 2];
 while (i <= j) {
   while (values[i] < pivot) {
     i++;
   }
   while (values[j] > pivot) {
     j--;
   }
   if (i <= j) {
     exValue(i, j);
     i++;
     j--;
   }
 }
 if (loIdx < j)
   qSort(loIdx, j);
 if (i < highIdx)
   qSort(i, highIdx);
}
void sortValues() {
  Serial.println("sortValues()");
  qSort(0, sampleCount - 1); 
}

// über n Sinus (obere Halbwelle) sampeln
void sampleValues() {
  Serial.println("sampleValues()");
  // hintereinander weg samplen, ohne Unterbrechung
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    values[i] = analogRead(ANALOG_IN);
  } 
}

// Messen peak bzw. high
int getMeasurement() {
  Serial.println("getMeasurement()");

  sampleValues();

  yield();
  ESP.wdtFeed();

  /* einfach den Max wert suchen
  int measurementValue = 0; 
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    if (values[i] > measurementValue) {
      measurementValue = values[i];
    }
  }
  return measurementValue;
  */

  sortValues();
  // die oberen n values wegschmeißen, ausreißer
  // und über die darauf folgenden values mitteln
  int measurementValue = 0; 
  int measurementValueCount = 0;
  for (int i = sampleCount-1-skipValues-measureValues; i < sampleCount-1-skipValues ; i++) 
  {
    measurementValue += values[i];
    measurementValueCount += 1;
  }
  measurementValue = measurementValue / measurementValueCount;
  return measurementValue;
}

// Debugging
String getValuesDebug() {
  Serial.println("getValuesDebug()");
  String valuesStr = "";

  sampleValues();
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    valuesStr += values[i];
    valuesStr += ";";
  }  
  valuesStr += "\r\n";

  sortValues();
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    valuesStr += values[i];
    valuesStr += ";";
  }  
  valuesStr += "\r\n";

  return valuesStr;
}

// --------------------------------------------

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

void handleCurrentSampleValues() {
  Serial.println("handleCurrentSampleValues()");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  String message = String(getValuesDebug());

  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void handleCurrentMeasurement() {
  Serial.println("handleCurrentMeasurement()");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  String message = String(getMeasurement());

  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void handleCurrentPower() {
  Serial.println("handleCurrentPower");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  int value = getMeasurement();
  Serial.println("Value:");
  Serial.println(value);
  float power = float(CALCURRENTPOWER)/float(CALCURRENTVALUE) * value;
  Serial.println("Power");
  Serial.println(power);

  String message = String(power);
  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

// --------------------------------------------

void handleAvgDo() {
  Serial.println("handleAvgDo");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  // nur das Flag setzen
  avgActive = true;

  String message = "OK";
  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH  
}

void doAvgMeasurement() {
  Serial.println("doAvgMeasurement");

  for (int i = 0; i < 3; i++) 
  {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH 
    delay(100);
  } 

  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  avgValue = 0;
  for (int i = 0; i <= (avgCount-1); i++) 
  {
      avgValue += getMeasurement();

      yield();    
      ESP.wdtFeed(); 
  }
  Serial.println("done");
  Serial.println(avgCount);
  Serial.println(avgValue);
  avgValue = avgValue / avgCount;
  Serial.println(avgValue);

  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH  
}

void handleAvgMeasurement() {
  Serial.println("handleAvgMeasurement()");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  String message = String(avgValue);

  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void handleAvgPower() {
  Serial.println("handleAvgPower");
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  int value = avgValue;
  Serial.println("Value:");
  Serial.println(value);
  float power = float(CALCURRENTPOWER)/float(CALCURRENTVALUE) * value;
  Serial.println("Power");
  Serial.println(power);

  String message = String(power);
  server.send(200, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

// --------------------------------------------

void startWifi(void) {
  Serial.println("StartWifi");

  WiFi.mode(WIFI_STA);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Connect
  WiFi.begin(ssid, password);
  Serial.println("Connecting Wifi");

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
}

void stopWifi(void) {
  Serial.println("stopWifi");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  Serial.begin(115200);
  Serial.println("Serial Monitor");

  // Watchdog
  // Es gibt einen hardware und einen software watchdog
  ESP.wdtDisable();     // Softwarewatchdog aus  
  ESP.wdtEnable(5000);  // Softwarewatchdog auf n sek stellen, achtung wenn ich ihn abgeschaltet lasse löst aller 6-7 Sekunden der Hardware watchdog aus

  // Wifi
  startWifi();

  // MDNS
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // Server
  server.on("/", handleRoot);

  server.on("/currentSampleValues", handleCurrentSampleValues);
  server.on("/currentMeasurement", handleCurrentMeasurement);
  server.on("/currentPower", handleCurrentPower);

  // Mittelwertmessung ausführen
  server.on("/avgDo", handleAvgDo);
  // Mittelwertmessung abfragen
  server.on("/avgFetchMeasurement", handleAvgMeasurement);
  server.on("/avgFetchPower", handleAvgPower);
 
  /*
  server.on("/test", []() {
    server.send(200, "text/plain", );
  });
  */

  server.begin();
  Serial.println("HTTP server started");
}

// --------------------------------------------

void loop(void) {
  // Main Loop
  // der server.handleClient() muss permanent aufgerufen werden
  // Blockieren länger als 20ms wird nicht empfohlen

  MDNS.update();
  server.handleClient();

  if (avgActive == true ) {
    avgActive = false;
    Serial.println("enter main loop hook");
 
    //delay(1000); // yield() im Hintergrund, abarbeitung 
    //stopWifi();

    // führt zur Wifi Unterbrechung
    // wenn reconect gesetzt verbindet er automatisch 
    // ansonsten müsste hier das wifi manuell gestoppt und neu gestartet werden
    doAvgMeasurement();

    //startWifi();

    Serial.println("leave main loop hook");
  }

}
