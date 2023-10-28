uint8_t soilPin = 0;   //one nail goes to +5V, the other nail goes to this analogue pin
int moisture_val;

int sensorVCC = 13;   //Set the pin providing power
int counter = 0;    //Inialize the parameter for counting

void setup() {
  Serial.begin(115200);     //open serial port
  pinMode(sensorVCC, OUTPUT);    // Inialize the pin
  digitalWrite(sensorVCC, LOW);    // Let the pin wait for counter 
}

void loop() {
  counter++;
  if(counter> 6){      // change this value to set "not powered" time. higher number bigger gap

    // power the sensor
    digitalWrite(sensorVCC, HIGH);
    delay(1000);

    // read the value from the sensor:
    // "analogRead" function gets the voltage value of analog input pin, so high value represents small resistance between two nails
    moisture_val = analogRead(soilPin);

    //stop power
    digitalWrite(sensorVCC, LOW);

    //Output resultss
    Serial.print("sensor = " );                       
    Serial.println(moisture_val); 
    delay(100);
    
    //Reset Counter
    counter=0;    
  }  
}
