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
const char *topic_humidity = "UoP/CO/326/E18/19/DHT11/humidity";
const char *topic_airquality = "UoP/CO/326/E18/19/MQ135/AirQuality";
const char *topic_LEDGREEN = "UoP/CO/326/E18/19/LEDGREEN";
const char *topic_LEDYELLOW = "UoP/CO/326/E18/19/LEDYELLOW";
const char *topic_LEDRED = "UoP/CO/326/E18/19/LEDRED";
const char *topic_LED = "UoP/CO/326/E18/19/LED";
const char *topic_buzzer= "UoP/CO/326/E18/19/Buzzer";
const char *topic = "UoP/CO/326/E18/19/";
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
 client.setCallback(BuzzerControlByMessage); 
 client.setCallback(LEDControlByMessage); 
 
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
 client.publish(topic, "Hello From ESP8266!");
 client.subscribe(topic_LED);
 client.subscribe(topic_buzzer);
}

//-----------------------------------------------------------------------------------------------//
//DHT11 sensor
void dht11(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  delay(10);

  if(isnan(h)||isnan(t)){
    Serial.println("Error: Failed to read from the dht sensor");
    return;
  }

  delay(100);
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
  client.publish(topic_airquality,payloada);
  delay(100);

}

//----------------------------------------------------------------------------------------------------------//
void LEDControlByMessage(char *topic, byte *payload, unsigned int length) {
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
  } 
 else if (ledcontrol == 2){
     digitalWrite(LEDYELLOW, HIGH); 
     digitalWrite(LEDGREEN, LOW);
     digitalWrite(LEDRED, LOW);   
  } 
 else if (ledcontrol == 3){
     digitalWrite(LEDRED, HIGH);   
     digitalWrite(LEDYELLOW, LOW);
     digitalWrite(LEDGREEN, LOW);   
  } 
  delay(1000); 
  ledcontrol = 0; 
  
 }

 if (String(topic)==topic_buzzer){
  for (int i = 0; i < length; i++) {
  Serial.print((char) payload[i]);
  messageInfo_buzzer += (char)payload[i];
 }
 int buzzercontrol = messageInfo_buzzer.toInt();
 if (buzzercontrol == 4){
     tone(BUZZER, 10000,500);
     delay(100);
     noTone(BUZZER);
     delay(100);        
  } 
  buzzercontrol = 0;
 }

 delay(1000);
   
}

void loop() {
 client.loop();
 dht11();
 mq135();
 delay(1000);  // Delay for one millisecond before taking the next reading
}
