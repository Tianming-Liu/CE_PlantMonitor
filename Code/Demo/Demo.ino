// Libraries for internet connection
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>

// Library for Real Time
#include <ezTime.h>

// Library for MQTT Connection
#include <PubSubClient.h>

// Libraries for DHT Sensor
#include <DHT.h>
#include <DHT_U.h>

// Functions from external file
#include <functions.cpp>

// Set up Wifi Connection and MQTT Connection from external secret file
#include <passwords.h>  // Here i write my setting and passwords in this "passwords.h"
/* You can also enter the sensitive data using following format:
#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
*/

// Set Wifi and MQTT Params From File Imported
const char* ssid     = SECRET_SSID;  // Wifi Name
const char* password = SECRET_PASS;  // Wifi Password
const char* mqttuser = SECRET_MQTTUSER;  // MQTT UserName (if you use the port needs authentication)
const char* mqttpass = SECRET_MQTTPASS;  // MQTT Password (if you use the port needs authentication)

// Initialize Web Server on ESP8266
ESP8266WebServer server(80);

// Set MQTT Broker Host
const char* mqtt_server = "mqtt.cetools.org"
// Set MQTT Client
WiFiClient espClient;  // Initialize Wifi Connection
PubSubClient client(espClient);  // Initialize MQTT Client using wifi connection

//
long lastMsg = 0;
char msg[50];
int value = 0;

// Initialize Date and time object
Timezone GB;

// Set DHT Sensor Type
#define DHTTYPE DHT22 

// Set up Digital Pin Number
uint8_t DHTPin = 12; // Pin for DHT sensor
uint8_t soilPin = 0; // Pin for read voltage

int sensorVCC = 13; // Set up digital pin for provide power for plant sensor
int blueLED = 2;

// Declare params
float Temperature; // Declare a param for temp
float Humidity;  // Declare a param for humidity
int Moisture = 1; // Declare and initialize a param

// Create a DHT Sensor object
DHT dht(DHTPin, DHTTYPE);  

void setup() {
  // Set up LED Pin
  pinMode(BUILTIN_LED, OUTPUT);  // Use the default LED Pin
  digitalWrite(BUILTIN_LED, HIGH);
  // Set up Soil Mositure Module Pin
  pinMode(sensorVCC, OUTPUT); 
  digitalWrite(sensorVCC, LOW);
  // 
  pinMode(blueLED, OUTPUT); 
  digitalWrite(blueLED, HIGH);

  // Set up serial connection
  Serial.begin(115200);
  delay(100);

  // Initialize DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();

  // run initialisation functions
  startWifi();
  startWebserver();
  syncDate();

  // start MQTT server
  client.setServer(mqtt_server, 1884);  // Pay attention to the port you use to connect the broker
  client.setCallback(callback);


}

void loop() {
  // handler for receiving requests to webserver
  server.handleClient();

  if (minuteChanged()) {
    readMoisture();
    sendMQTT();
    Serial.println(GB.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")
  }

  client.loop();
}
