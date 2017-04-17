#include "Print.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include "RTClib.h"

#define DHTPIN  2         // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define DATA_ERROR F("Error")
#define ROOT_TOPIC F("arduino/")

DHT_Unified dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

uint32_t delayMS;
String topic = "" ;
String data = "";
String date;
int qos = 2;
bool retain = false;

void MQTToSerial_pub(String top, String payload, int q, bool ret) {
  
  String t = "";
  t.concat(ROOT_TOPIC);
  t.concat(top);
  
  Serial.print(F("{'topic':'"));
  Serial.print(t);
  Serial.print(F("', 'payload':'"));
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
  delay(100);
  Serial.println("Booting...");
  Serial.print("RTC...");
  
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
  Serial.println("OK!");
  
  //Print details about date and time
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
  
  MQTToSerial_pub(F("last_start"), date, 2, true);
  
  // Initialize DHT22 sensor
  dht.begin();

  // Print temperature sensor details.
  sensor_t sensor;

  // Send sensors details
  dht.temperature().getSensor(&sensor);

  MQTToSerial_pub(F("DHT22/temperature/sensor"),     sensor.name,               2, true);
  MQTToSerial_pub(F("DHT22/temperature/version"),    String(sensor.version),    2, true);
  MQTToSerial_pub(F("DHT22/temperature/sensor_id"),  String(sensor.sensor_id),  2, true);
  MQTToSerial_pub(F("DHT22/temperature/max_value"),  String(sensor.max_value),  2, true);
  MQTToSerial_pub(F("DHT22/temperature/min_value"),  String(sensor.min_value),  2, true);
  MQTToSerial_pub(F("DHT22/temperature/resolution"), String(sensor.resolution), 2, true);
  
  dht.humidity().getSensor(&sensor);

  MQTToSerial_pub(F("DHT22/humidity/sensor"),      sensor.name,                2, true);
  MQTToSerial_pub(F("DHT22/humidity/version"),     String(sensor.version),     2, true);
  MQTToSerial_pub(F("DHT22/humidity/sensor_id"),   String(sensor.sensor_id),   2, true);
  MQTToSerial_pub(F("DHT22/humidity/max_value"),   String(sensor.max_value),   2, true);
  MQTToSerial_pub(F("DHT22/humidity/min_value"),   String(sensor.min_value),   2, true);
  MQTToSerial_pub(F("DHT22/humidity/resolution"),  String(sensor.resolution),  2, true);
  
  MQTToSerial_pub(F("DHT22/min_delay"),            String(sensor.min_delay),   2, true);
  
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
  
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  
  if (isnan(event.temperature)) data = DATA_ERROR;
  else data = event.temperature;

  topic = "DHT22/temperature/date";
  MQTToSerial_pub(topic, date, 2, true);
  topic = "DHT22/temperature/value";
  MQTToSerial_pub(topic, data, 2, true);
  
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) data = DATA_ERROR;
  else data = event.relative_humidity;

  topic = "DHT22/humidity/date";
  MQTToSerial_pub(topic, date, 2, true);
  topic = "DHT22/humidity/value";
  MQTToSerial_pub(topic, data, 2, true);
  
}
