#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>  // Including library for dht
#include "MQ135.h"

//PINS
#define DHTPIN D7          
//#define MQ135D D7
#define MQ135A A0 
#define LEDGREEN D1
#define LEDYELLOW D2
#define LEDRED D3
#define BUZZER D4



//define sensors
DHT dht(DHTPIN, DHT11);
MQ135 gasSensor = MQ135(A0); 
    

// variables
const char *ssid = "Eng-Student"; // Enter your WiFi name
const char *password = "3nG5tuDt";  // Enter WiFi password
// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org"; // Enter your WiFi or Ethernet IP
const char *topic_temp = "UoP/CO/326/E18/19/DHT11/temperature";
const char *topic_humidity = "UoP/CO/326/E18/19/DHT11/humidity";
const char *topic_airquality = "UoP/CO/326/E18/19/MQ135/AirQuality";
const char *topic_LEDGREEN = "UoP/CO/326/E18/19/LEDGREEN";
const char *topic_LEDYELLOW = "UoP/CO/326/E18/19/LEDYELLOW";
const char *topic_LEDRED = "UoP/CO/326/E18/19/LEDRED";
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
  float air_quality = gasSensor.getRZero();
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

}

void GREENLEDControl(){
  int control=0;
  control = client.subscribe(topic_LEDGREEN);
  if (control==1){
    digitalWrite(LEDGREEN, HIGH);   
    delay(1000);              
    digitalWrite(LEDGREEN, LOW);       
  }
}

void YELLOWLEDControl(){
  int control=0;
  control = client.subscribe(topic_LEDYELLOW);
  if (control==2){
    digitalWrite(LEDYELLOW, HIGH);   
    delay(1000);              
    digitalWrite(LEDYELLOW, LOW);       
  }
}

void REDLEDControl(){
  int control=0;
  control = client.subscribe(topic_LEDRED);
  if (control==3){
    digitalWrite(LEDRED, HIGH);   
    delay(100);              
    digitalWrite(LEDRED, LOW);       
  }
}

void BUZZERControl(){
  int control=0;
  control = client.subscribe(topic_LEDRED);
  if (control==4){
    tone(BUZZER, 10000,500);
    delay(1000);
    noTone(BUZZER);
    delay(1000);     
  }
}
void loop() {
 client.loop();
 dht11();
 mq135();
 GREENLEDControl();
 YELLOWLEDControl();
 REDLEDControl();
 BUZZERControl();
 delay(1000);  // Delay for one second before taking the next reading
}
