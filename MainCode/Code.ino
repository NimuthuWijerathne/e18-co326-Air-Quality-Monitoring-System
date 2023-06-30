#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>  // Including library for dht
#include "MQ135.h"

//PINS
#define DHTPIN D7          
//#define MQ135D D7
#define MQ135A A0 
#define LEDGREEN 5 //d1
#define LEDYELLOW 4 //d2
#define LEDRED 0 //d3
#define BUZZER 2 //d4

//define sensors
DHT dht(DHTPIN, DHT11);
MQ135 gasSensor = MQ135(A0); 
    
// variables
const char *ssid = "vivo"; // Enter your WiFi name
const char *password = "nimu123456";  // Enter WiFi password
// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org"; 
const char *topic_temp = "UoP/CO/326/E18/19/DHT11/temperature";
const char *topic_tempF = "UoP/CO/326/E18/19/DHT11/temperatureF";
const char *topic_humidity = "UoP/CO/326/E18/19/DHT11/humidity";
const char *topic_airquality = "UoP/CO/326/E18/19/MQ135/AirQuality";
//const char *topic_LEDGREEN = "UoP/CO/326/E18/19/LEDGREEN";
//const char *topic_LEDYELLOW = "UoP/CO/326/E18/19/LEDYELLOW";
//const char *topic_LEDRED = "UoP/CO/326/E18/19/LEDRED";
const char *topic_LED = "UoP/CO/326/E18/19/LED";
const char *topic_buzzer= "UoP/CO/326/E18/19/Buzzer";
const char *topic_general = "UoP/CO/326/E18/19/";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
 // Set software serial baud to 115200;
 Serial.begin(115200);
 delay(10);
 dht.begin();
 pinMode(LEDGREEN, OUTPUT);
 pinMode(LEDYELLOW, OUTPUT);
 pinMode(LEDRED, OUTPUT);
 pinMode(BUZZER, OUTPUT);
 
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
    delay(1000);
 }
} 
 // publish and subscribe
 client.publish(topic_general, "Hello From ESP8266!");
 client.subscribe(topic_LED);
 client.subscribe(topic_buzzer);
}

//-----------------------------------------------------------------------------------------------//
//DHT11 sensor
void dht11(){
  float humidity = dht.readHumidity();
  float TempC = dht.readTemperature();
  float TempF = 0;
  TempF = (TempC*1.8)+32;
  delay(10);

  if(isnan(humidity)||isnan(TempC)){
    Serial.println("Error: Failed to read from the dht sensor");
    return;
  }

  delay(100);
  Serial.print("Temperature: ");
  char payloadtempc[10];
  dtostrf(TempC, 4, 2, payloadtempc);
  Serial.print(TempC);
  char payloadtempf[10];
  dtostrf(TempF, 4, 2, payloadtempf);
  Serial.print(" degrees Celcius, Humidity: ");
  char payloadh[10];
  dtostrf(humidity, 4, 2, payloadh);
  Serial.print(humidity);
  Serial.println();

  client.publish(topic_temp,payloadtempc);
  client.publish(topic_humidity,payloadh);
  client.publish(topic_tempF,payloadtempf);

}
//---------------------------------------------------------------------------------------------------------------//

//MQ135 sensor
void mq135(){
  float air_quality = gasSensor.getPPM();
   if(isnan(air_quality)){
    Serial.println("Error: Failed to read from the MQ135 sensor");
    return;
  }
  delay(10);

  Serial.print("Air Quality: ");  
  Serial.print(air_quality);
  char payloada[10];
  dtostrf(air_quality, 4, 2, payloada);
  Serial.println("  PPM");   
  Serial.println();
  client.publish(topic_airquality, payloada);
  delay(100);

}

//----------------------------------------------------------------------------------------------------------//
void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 String messageInfo_LED;
 String messageInfo_buzzer;

 if (String(topic)==topic_LED){
  for (int i = 0; i < length; i++) {
  Serial.print((char) payload[i]);
  messageInfo_LED += (char)payload[i];
 }

 int ledcontrol = messageInfo_LED.toInt();
 Serial.println(" - - - - - - - - - - - -");

 if (ledcontrol == 1){
     digitalWrite(LEDGREEN, HIGH);
     digitalWrite(LEDYELLOW, LOW);
     digitalWrite(LEDRED, LOW); 
     delay(100);        
  } 
 else if (ledcontrol == 2){
     digitalWrite(LEDYELLOW, HIGH); 
     digitalWrite(LEDGREEN, LOW);
     digitalWrite(LEDRED, LOW);
     delay(100);   
  } 
 else if (ledcontrol == 3){
     digitalWrite(LEDRED, HIGH);   
     digitalWrite(LEDYELLOW, LOW);
     digitalWrite(LEDGREEN, LOW); 
//     tone(BUZZER, 5000,1000);
//     delay(1000);
//     noTone(BUZZER);
//     delay(500); 
  }
  delay(200); 
  ledcontrol = 0;  
 }

 if (String(topic)==topic_buzzer){
  char buzzercontrol[length+1];
  memcpy(buzzercontrol, payload, length);
  buzzercontrol[length] = '\0';
  if (strcmp(buzzercontrol, "buzzeron")==0){
     tone(BUZZER, 5000,1000);
     delay(100);}
 else{
     noTone(BUZZER);
     delay(100);        
  } 
//  buzzercontrol = "no";
  delay(200);
   
 }
}

void loop() {
 client.loop();
 dht11();
 mq135();
 delay(500);  // Delay before taking the next reading
}
