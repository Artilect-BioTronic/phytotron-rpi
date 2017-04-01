#include "Print.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include "RTClib.h"

#define DHTPIN  2         // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define DATA_ERROR F("Error")

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

uint32_t delayMS;
String topic ;
String data = "";
String date;
int qos = 2;
bool retain = false;

void MQTToSerial_pub(String top, String payload, int q, bool ret) {
  Serial.print(F("{'topic':'"));
  Serial.print(top);
  Serial.print(F("', 'data':'"));
  Serial.print(payload);
  Serial.print(F("', 'qos':'"));
  Serial.print(q);
  Serial.print(F("', 'retain':'"));
  Serial.print(ret);
  Serial.println(F("'}"));
}

void MQTToSerial_sub() {
  // TODO
}

void setup() {
  Serial.begin(115200);

  // Initialize RTC DS1307
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  //Print details about date and time
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  
  // Initialize DHT22 sensor
  dht.begin();

  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Humidity"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}

void loop() {
  // Delay between measurements.
  delay(delayMS);

  //Get date & time
  DateTime now = rtc.now();
  date = "";
  date.concat(now.year());
  date.concat('/');
  date.concat(now.month());
  date.concat('/');
  date.concat(now.day());
  date.concat('_');
  date.concat(now.hour());
  date.concat(':');
  date.concat(now.minute());
  date.concat(':');
  date.concat(now.second());
  date.concat(':');
  
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  
  if (isnan(event.temperature)) data = DATA_ERROR;
  else data = event.temperature;

  topic = "arduino/DHT22/temperature/date";
  MQTToSerial_pub(topic, date, 2, true);
  topic = "arduino/DHT22/temperature/value";
  MQTToSerial_pub(topic, data, 2, true);
  
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) data = DATA_ERROR;
  else data = event.relative_humidity;

  topic = "arduino/DHT22/humidity/date";
  MQTToSerial_pub(topic, date, 2, true);
  topic = "arduino/DHT22/humidity/value";
  MQTToSerial_pub(topic, data, 2, true);
  
}
