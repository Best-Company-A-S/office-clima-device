#include "DHT.h"

#define DHT22_PIN 12
#define DHTTYPE DHT22 

DHT dht(DHT22_PIN, DHTTYPE);

// Test device ID: 6fe26f8eaf7e

const int TemperatureAlarmLED =  9;
const int HumidityAlarmLED =  6;

int TemperaturePin = LOW;
int HumidityPin = LOW;


void setup(){

  pinMode(TemperatureAlarmLED, OUTPUT);
  pinMode(HumidityAlarmLED, OUTPUT);

  digitalWrite(TemperatureAlarmLED, LOW);
  digitalWrite(HumidityAlarmLED, LOW);
  
  Serial.begin(9600); 

  dht.begin();
}


void loop()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  Serial.print("Temperature = "); Serial.println(temperature);
  Serial.print("Humidity = "); Serial.println(humidity);
  delay(2500);

  if (temperature > 25) {
  TemperaturePin = HIGH;
  digitalWrite(TemperatureAlarmLED, HIGH);
  delay(1000);
  }

  if (temperature < 25) {  
  TemperaturePin = LOW;
  digitalWrite(TemperatureAlarmLED, LOW);
  }

  if (humidity > 55) {
  HumidityPin = HIGH;
  digitalWrite(HumidityAlarmLED, HIGH);
  delay(1000);
  }

  if (humidity < 55) {  
  HumidityPin = LOW;
  digitalWrite(HumidityAlarmLED, LOW);
  }
  
}