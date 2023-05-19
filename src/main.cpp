#include <Arduino.h>
#include "secrets.h"
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define DEVICE 'ESP'

WiFiMulti wifiiMulti;

// set timezone string UTC
#define TZ_INFO "EAT-3" //nairobi

// create an influxDB client instance, it will generate a secure client with 
// preconfigured certificate
InfluxDBClient client(
  INFLUXDB_URL,
  INFLUXDB_TOKEN,
  INFLUXDB_ORG,
  INFLUXDB_BUCKET,
  InfluxDbCloud2CACert
);

// create a datapoint (voltage_status)
Point sensor("voltageStatus"); // measurement, tagSet, fieldKey, field_Value, timeStamp

// pin definition
const int AnalogChannelPin = 15;
int ADCValue = 0; // stores ADC value
float voltageValue; // stores the voltage value

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // set esp32 in station modes
  wifiiMulti.addAP(
    WIFI_SSID,
    WIFI_PASSWORD
  );
  // make use of the "run" method to connect ESP32 to WiFi.
  Serial.print("Connecting to WiFi ...");
  while (wifiiMulti.run() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  // tagging the data
  sensor.addTag("device", DEVICE);
  sensor.addTag("ssid", WiFi.SSID());

}