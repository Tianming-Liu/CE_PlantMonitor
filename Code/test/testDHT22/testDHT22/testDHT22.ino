#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT22  // Sensor Type

uint8_t DHTPin = 12; // Set digital pin on Huzzah connected to DHT22
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

DHT dht(DHTPin,DHTTYPE); //Initialize DHT sensor object

void setup() {
  // Open serial connection and set port speed
  Serial.begin(115200);
  delay(100);
  // Start DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();
}

void loop() {

  // Get humidity from sensor
  float h = dht.readHumidity();
  // Get temperature as Celsius 
  float t = dht.readTemperature();
  // Get temperature as Fahrenheit (Input true parameter)
  float f = dht.readTemperature(true);

  // Check if there is any error when reading data
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // for clear print different value in serial monitor

  // print humidity and change line
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  
  // print Celsius Temperature
  Serial.print("Celsius Temperature: ");
  Serial.print(t);
  Serial.print("°C  ");
  
  // print Fahrenheit Temperature and change line
  Serial.print("Fahrenheit Temperature: ");
  Serial.print(f);
  Serial.println("°F");
  
  // print heat index (real feel temperature) in Celsius
  Serial.print("Celsius Heat Index: ");
  Serial.print(dht.computeHeatIndex(t,h,false));
  Serial.print("°C  ");

  delay(1000);
}

