#include <ThingSpeak.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFi.h>

int selectPins[3] = {25, 13, 26};
int readPins[2] = {32, 33};

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

int red = 12;
int yellow = 14;
int green = 27;

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
//  pinMode(18, OUTPUT);
//  pinMode(19, OUTPUT);
//  pinMode(21, OUTPUT);
//  pinMode(36, INPUT);
//  pinMode(39, INPUT);
//  pinMode(34, INPUT);
//  pinMode(35, INPUT);

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);

  digitalWrite(red, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(green, LOW);
}

int do_ = 0;
void loop() {
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
//   put your main code here, to run repeatedly:
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
    sensorData += String(Data[i]) + ",";
  }
  Serial.println(sensorData);
  String dataString = "field1=" + sensorData;
  String topicString ="channels/" + String(writeChannelID) + "/publish";
  
  mqttClient.publish( topicString.c_str(), dataString.c_str() );

  delay(80);
}
