// Libraries for internet connection
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Library for Real Time
#include <ezTime.h>

// Library for MQTT Connection
#include <PubSubClient.h>

// Libraries for DHT Sensor
#include <DHT.h>
#include <DHT_U.h>

#include "config.h"
// Functions from external file
// #include "functions.cpp"

// Set up Wifi Connection and MQTT Connection from external secret file
#include "passwords.h"  // Here i keep my setting and passwords in this "passwords.h"
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
const char* mqtt_server = "mqtt.cetools.org";
// Set MQTT Client
WiFiClient espClient;  // Initialize Wifi Connection
PubSubClient client(espClient);  // Initialize MQTT Client using wifi connection

// //
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

  if (secondChanged(15)) {
    readMoisture();
    sendMQTT();
    Serial.println(GB.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")
  }

  client.loop();
}

/*
-------------------------------------------------------------------------Function Definition--------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

bool secondChanged(int interval) {
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis >= (interval * 1000)) {
    lastMillis = currentMillis;
    return true;
  }
  return false;
}

void readMoisture(){

  // power the sensor
  digitalWrite(sensorVCC, HIGH);
  digitalWrite(blueLED, LOW);
  delay(100);
  // read the value from the sensor:
  Moisture = analogRead(soilPin);         
  //Moisture = map(analogRead(soilPin), 0,320, 0, 100);    // note: if mapping work out max value by dipping in water     
  //stop power
  digitalWrite(sensorVCC, LOW);  
  digitalWrite(blueLED, HIGH);
Í
  Serial.print("Wet ");
  Serial.println(Moisture);   // read the value from the nails
}

void startWifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // check to see if connected and wait until you are
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void syncDate() {
  // get real date and time
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  GB.setLocation("Europe/London");
  Serial.println("London time: " + GB.dateTime());
}

void startWebserver() {
  // when connected and IP address obtained start HTTP server
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void sendMQTT() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Temperature = dht.readTemperature(); // Gets the values of the temperature
  snprintf (msg, 50, "%.1f", Temperature);
  Serial.print("Publish message for t: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfntl0/temperature", msg);

  Humidity = dht.readHumidity(); // Gets the values of the humidity
  snprintf (msg, 50, "%.0f", Humidity);
  Serial.print("Publish message for h: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfntl0/humidity", msg);

  //Moisture = analogRead(soilPin);   // moisture read by readMoisture function
  snprintf (msg, 50, "%.0i", Moisture);
  Serial.print("Publish message for m: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfntl0/moisture", msg);

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect with clientID, username and password
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("student/CASA0014/plant/ucfntl0/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "text/html", SendHTML(Temperature, Humidity, Moisture));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat, float Humiditystat, int Moisturestat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP8266 DHT22 Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Huzzah DHT22 Report</h1>\n";

  ptr += "<p>Temperature: ";
  ptr += (int)Temperaturestat;
  ptr += " C</p>";
  ptr += "<p>Humidity: ";
  ptr += (int)Humiditystat;
  ptr += "%</p>";
  ptr += "<p>Moisture: ";
  ptr += Moisturestat;
  ptr += "</p>";
  ptr += "<p>Sampled on: ";
  ptr += GB.dateTime("l,");
  ptr += "<br>";
  ptr += GB.dateTime("d-M-y H:i:s T");
  ptr += "</p>";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
