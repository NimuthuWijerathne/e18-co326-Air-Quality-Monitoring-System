#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>  // Including library for dht
#include "MQ135.h"
#include <Arduino.h>


//PINS
#define DHTPIN D7          
//#define MQ135D D7
#define MQ135A A0 

//define sensors
DHT dht(DHTPIN, DHT11);
MQ135 gasSensor = MQ135(A0); 
   

// variables
const char *ssid = "vivo"; // Enter your WiFi name
const char *password = "nimu123456";  // Enter WiFi password
// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org"; // Enter your WiFi or Ethernet IP
const char *topic_temp = "326/grp19/sensors/DHT11/temp";
const char *topic_humidity = "326/grp19/sensors/DHT11/humi";
const char *topic_airquality = "326/grp19/sensors/MQ135/";
const char *topic_LED = "326/grp19/Actuators/LED/";
const char *topic_buzzer= "326/grp19/Actuators/Buzzer/";
const char *topic = "326/grp19/";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
 #define GLED D1 //Define green LED pin
 // Set software serial baud to 115200;
 Serial.begin(115200);
 delay(10);
 dht.begin();
 
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback); 
 while (!client.connected()) {
  String client_id = "esp8266-client-";
  client_id += String(WiFi.macAddress()); 
  Serial.printf("The client %s connects to mosquitto mqtt broker\n", client_id.c_str()); 
  if (client.connect(client_id.c_str())) {
    Serial.println("Public emqx mqtt broker connected");
  } else {
    Serial.print("failed with state ");
    Serial.print(client.state());
    delay(2000);
 }
} 

 // publish and subscribe
 client.publish(topic, "Hello From ESP8266!");
 client.subscribe(topic);
 
}

void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 
 for (int i = 0; i < length; i++) {
  Serial.print((char) payload[i]);
 }
 
 Serial.println();
 Serial.println(" - - - - - - - - - - - -");
}

//DHT11 sensor
void dht11(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  delay(500);

  if(isnan(h)||isnan(t)){
    Serial.println("Error: Failed to read from the dht sensor");
    return;
  }

  delay(1000);
  Serial.print("Temperature: ");
  char payloadt[10];
  dtostrf(t, 4, 2, payloadt);
  Serial.print(t);
  Serial.print(" degrees Celcius, Humidity: ");
  char payloadh[10];
  dtostrf(h, 4, 2, payloadh);
  Serial.print(h);
  Serial.println();

  client.publish(topic_temp,payloadt);
  client.publish(topic_humidity,payloadh);

}

//MQ135 sensor
void mq135(){
  float air_quality = gasSensor.getPPM();
   if(isnan(air_quality)){
    Serial.println("Error: Failed to read from the MQ135 sensor");
    return;
  }
  delay(500);

  Serial.print("Air Quality: ");  
  Serial.print(air_quality);
  char payloada[10];
  dtostrf(air_quality, 4, 2, payloada);
  Serial.println("  PPM");   
  Serial.println();

  client.publish(topic_airquality,payloada);
//  return payloada;

}

//void greenled(char payloada){
//
//  int recommendedAmount = 20000;
//
//  if(payloada>recommendedAmount){
//    digitalWrite(GLED, LOW); // Turn the LED on (Note that LOW is the voltage level)
//    delay(1000); // Wait for a second
//  }
//   
//}


void loop() {
 client.loop();
 dht11();
 mq135();
 delay(1000);  // Delay for one second before taking the next reading
// greenled(payloada);
}
