#include <ThingSpeak.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFi.h>

int selectPins[3] = {25, 13, 26};
int readPins[2] = {32, 33};
String out = "";

float Data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char ssid[] = "Coldspot";
char password[] = "Password";

const char server[] = "mqtt3.thingspeak.com";
char mqttUserName[] = "Aamras";
char mqttPass[] = "A3P7UKFEOYW3AGOM";

int writeChannelID = 2151405;
char writeAPIKey[] = "N52JQQ3SXWEFXHHR";

WiFiClient myClient;
PubSubClient mqttClient(server, 1883, myClient);

int red_led = 12;
int yellow_led = 14;
int green_led = 27;

int mode_ = 0;

int pb = 34;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("Connecting...");
    delay(100);  
  }
  Serial.println("Connected.");
  mqttClient.setServer(server, 1883);

  analogReadResolution(10); //default is 12. Can be set between 9-12.
  for(int i = 0; i < 3; i++){
    pinMode(selectPins[i], OUTPUT);
  }
  for(int i = 0; i < 2; i++){
    pinMode(readPins[i], INPUT);  
  }
  
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);

  digitalWrite(red_led, HIGH);
  digitalWrite(yellow_led, LOW);
  digitalWrite(green_led, LOW);

  pinMode(pb, INPUT);
}

int do_ = 0;

int minValues[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int avgValues[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int maxValues[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int calibrate = 1;

void loop() {
  if(calibrate){
    Serial.println("Starting to Calibrate. Put a cloth on the sensors (5 seconds))");
    delay(5000); // Wait 5 seconds to reset all esp32s

    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, HIGH);
    digitalWrite(green_led, LOW);
    
    for(int mux = 0; mux < 2; mux++){
    int readPin = readPins[mux];
    for(int j = 0; j < 8; j++){
      int temp = j;
      int s0 = temp % 2;
      temp = temp / 2;
      int s1 = temp % 2;
      temp = temp / 2;
      int s2 = temp % 2;

      digitalWrite(selectPins[0], s0);
      digitalWrite(selectPins[1], s1);
      digitalWrite(selectPins[2], s2);
      delay(5);
      float data_ = analogRead(readPin);
      minValues[mux * 8 + j] = data_;
      delay(5);
     }
  }
  for(int i = 0; i < 16; i++){
      out += String(minValues[i]) + ",";
  }
  Serial.println(out);
    Serial.println("Remove the cloth from the sensors (5 seconds))");
    delay(5000); // 5 seconds to remove the cloth

    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, HIGH);
    
    for(int mux = 0; mux < 2; mux++){
    int readPin = readPins[mux];
    for(int j = 0; j < 8; j++){
      int temp = j;
      int s0 = temp % 2;
      temp = temp / 2;
      int s1 = temp % 2;
      temp = temp / 2;
      int s2 = temp % 2;

      digitalWrite(selectPins[0], s0);
      digitalWrite(selectPins[1], s1);
      digitalWrite(selectPins[2], s2);
      delay(5);
      float data_ = analogRead(readPin);
      maxValues[mux * 8 + j] = data_;
      delay(5);
     }
  }

  out = "";
  for(int i = 0; i < 16; i++){
      out += String(maxValues[i]) + ",";
  }
  Serial.println(out);
  
  for(int i = 0; i < 16; i++){
    avgValues[i] = (maxValues[i] + minValues[i]) / 2;  
  }

  out = "";
  for(int i = 0; i < 16; i++){
      out += avgValues[i] + ",";
  }
    calibrate = 0;
    Serial.println("Calibration Stopped");
    delay(1000);
  }
  while(!mqttClient.connected())
  {
    Serial.println("Connect Loop");
    Serial.println(mqttClient.connected());
    // mqttConnect();
    // clientID, username, password
    mqttClient.connect("DAIhLy40OiYJNggHMDILAjQ", "DAIhLy40OiYJNggHMDILAjQ", "H9Sf69EqRKGVwkZK9PGU2kBu");
  }
  if(do_ == 0){
    Serial.println("MQTT Connected!");
    do_ += 1;
  }

  for(int mux = 0; mux < 2; mux++){
    int readPin = readPins[mux];
    for(int j = 0; j < 8; j++){
      int temp = j;
      int s0 = temp % 2;
      temp = temp / 2;
      int s1 = temp % 2;
      temp = temp / 2;
      int s2 = temp % 2;

      digitalWrite(selectPins[0], s0);
      digitalWrite(selectPins[1], s1);
      digitalWrite(selectPins[2], s2);
      delay(5);
      float data_ = analogRead(readPin);
      Data[mux * 8 + j] = data_;
      delay(5);
     }
  }
//  for(int i = 0; i < 12; i++){
//    Data[i] = analogRead(readPins[i]);  
//  }
  String sensorData = "";
  for(int i = 0; i < 16; i++){
    if(Data[i] < avgValues[i]){
      sensorData += "1,";
    } else{
      sensorData += "0,";
    }
  }
  Serial.println(sensorData);
  String dataString = "field1=" + sensorData;
  String topicString ="channels/" + String(writeChannelID) + "/publish";
  
  mqttClient.publish( topicString.c_str(), dataString.c_str() );

  delay(80);
}
