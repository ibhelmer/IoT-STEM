/*
 *  Simple example of end device to MQTT (using FireBeetle)
 *  Subscribing to Led1, Led2 and Led3 channel 
 *  Publiching on sw1 and an0
 *  Device ID is shown in I2C display
 *  UCN/IT-Techonlogy/Ib Helmer Nielsen/2021
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DFRobot_RGBLCD1602.h"

#define  MAX_RANG      (520)
#define  ADC_SOLUTION  (4095.0)

// Constants
// IoT Device
// Subscription and publish channel
#define      DEVID     "esp15"
#define      DEVCLIENT "esp15Client"
#define      LED1SUB   "esp15/Led1"
#define      LED2SUB   "esp15/Led2"
#define      LED3SUB   "esp15/Led3"
#define      SW1PUB    "esp15/sw1"
#define      AN0PUB    "esp15/an0"
// WiFi Setup
const char * ssid = "ITLab";
const char * passwd = "MaaGodt*7913";
// MQTT Broker address and port
const char * mqtt_server = "192.168.1.45";    // Ip adress of MQTT broker
const uint16_t mqtt_port = 1883;              // TCP port used by the MQTT broker
// LCD back light color
const int colorFull = 255;
const int colorHalf = 128;
const int colorNo = 0;
// LED
int led1Pin = D0;
int led2Pin = D2;
int led3Pin = D3;
// Switch
int sw1Pin = D7;
int sw2Pin = D9;
int sw3Pin = D10;
// Analog
int an0Pin = A0;

// Objects 
WiFiClient esp32;                                     // WiFi
PubSubClient client(esp32);                           // MQTT client object
DFRobot_RGBLCD1602 lcd(/*lcdCols*/16,/*lcdRows*/2);   //16 characters and 2 lines of show

// Variables
long lastMsg = 0;
char msg[5];  
int value = 0;

void setup()
{
    Serial.begin(115200);
    lcd.init();
    setup_wifi();
    pinMode(led1Pin, OUTPUT); digitalWrite(led1Pin, LOW);
    pinMode(led2Pin, OUTPUT); digitalWrite(led2Pin, LOW);
    pinMode(led3Pin, OUTPUT); digitalWrite(led3Pin, LOW);
    pinMode(sw1Pin,INPUT);  
    pinMode(sw2Pin,INPUT);
    pinMode(sw3Pin,INPUT);
    lcd.setRGB(colorNo, colorHalf, colorNo);
    lcd.print(WiFi.localIP());
    lcd.setCursor(0,2);
    lcd.print("Dev ID: ");
    lcd.print(DEVID); 
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void setup_wifi()
{  
  delay(10);
  // Connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, passwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  

  if (String(topic) == String(LED1SUB)) {  
    Serial.print("Led1 changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");    
      digitalWrite(led1Pin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(led1Pin, LOW);
    }
  }
  if (String(topic) == String(LED2SUB)) {
    Serial.print("Led2 changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");    
      digitalWrite(led2Pin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(led2Pin, LOW);
    }
  }
  if (String(topic) == String(LED3SUB)) {
    Serial.print("Led3 changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");    
      digitalWrite(led3Pin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(led3Pin, LOW);
    }
  }
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    lcd.setRGB(colorHalf, colorNo, colorNo); // Display red if not connected to broker
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVCLIENT)) {
      Serial.println("connected");
      // Subscribe to channel
      client.subscribe(LED1SUB);
      client.subscribe(LED2SUB);
      client.subscribe(LED3SUB);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  lcd.setRGB(colorNo, colorHalf, colorNo); // Display geeen if connected to broker
}

float dist_t, sensity_t;
int sw1State = 0;
int lastSw1State = 0;
String an0txt;
char dist[10];

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
  int val = digitalRead(sw1Pin);
  if (val==HIGH) { 
    Serial.print("*");
    client.publish(SW1PUB,"on");
  }
  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now; 
    sensity_t = analogRead(an0Pin);
    dist_t = sensity_t * 3.55 * MAX_RANG  / ADC_SOLUTION;//
    Serial.println(dist_t,0);
    an0txt = String(dist_t);
    an0txt.toCharArray(dist,an0txt.length()+1);
    client.publish(AN0PUB,dist);
    sw1State = digitalRead(sw1Pin);
    if (sw1State != lastSw1State)
    {
       if (sw1State==HIGH)
       {
         client.publish(SW1PUB,"on");
       } else {
         client.publish(SW1PUB,"off");
       } 
       lastSw1State = sw1State;
    }
  }
}
