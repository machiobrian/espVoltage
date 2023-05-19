#include <Arduino.h>
#include "secrets.h"
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

WiFiMulti wifiMulti;

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
const int AnalogChannelPin = 32;
int ADCValue = 0; // stores ADC value
float voltageValue; // stores the voltage value

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // set esp32 in station modes
  wifiMulti.addAP(
    WIFI_SSID,
    WIFI_PASSWORD
  );
  // make use of the "run" method to connect ESP32 to WiFi.
  Serial.print("Connecting to WiFi ...");
  while (wifiMulti.run() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  // tagging the data
  sensor.addTag("device", DEVICE);
  sensor.addTag("ssid", WiFi.SSID());

  // sync time with the NTP server
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  if (client.validateConnection()){
    Serial.print("InfluxDB:Connected ");
    Serial.println(client.getServerUrl());
  }else{
    Serial.print("InfluxDB:NoConnection ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop(){
  // before elogging data.
  sensor.clearFields(); //clear all fields of the point instance

  // read the analog input
  ADCValue = analogRead(AnalogChannelPin);
  delay(200);
  voltageValue = (ADCValue*3.3)/(4095);
  delay(200);
  sensor.addField("voltage", voltageValue);
  Serial.println(client.pointToLineProtocol(sensor));

  // incase of WiFi Disconnection
  if (wifiMulti.run() != WL_CONNECTED){
    Serial.println("WiFi Conn Lost");
  }

  // write the point to the database every 5 seconds
  if(!client.writePoint(sensor)){
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  delay(500);
}