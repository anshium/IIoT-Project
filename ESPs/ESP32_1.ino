#include <HTTPClient.h>
#include <WiFi.h>
#include "time.h"
#include <ThingSpeak.h>
#include <PubSubClient.h>
#include <WiFiClient.h>


#define MAIN_SSID "Galaxy M31FCCB"
#define MAIN_PASS "pvub5546"

#define CSE_IP "192.168.57.64"
#define CSE_PORT 5089
#define OM2M_ORGIN "admin:admin"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "SmartParkingSystem"
#define NODE "SubStation-1"
#define NODE_NUMBER 1
#define OM2M_DATA_CONT "Data"

#define N 6

String node1[N] = { "i3", "i4", "i5", "i6", "B1", "B2" };

int inputPins[N] = { 32, 33, 34, 35, 36, 39 };

int Data[N] = { 0, 0, 0, 0, 0, 0 };


#define username "FwYNDA4VCzkuMigUNjwXMCM"
#define clientID "FwYNDA4VCzkuMigUNjwXMCM"
#define password "ppieJfYd1hSevcdgpXYrDP2H"

const char server[] = "mqtt3.thingspeak.com";
int writeChannelID = 2167101;
char writeAPIKey[] = "N2DPU8YFUT8TZ4AV";

const char* ntpServer = "pool.ntp.org";
unsigned long epochTime;


int red_led = 5;
int yellow_led = 18;
int green_led = 19;


WiFiClient myClient;
HTTPClient http;

PubSubClient mqttClient(server, 1883, myClient);


unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
    return (0);

  time(&now);
  return now;
}

void setup() {
  Serial.begin(9600);
  configTime(0, 0, ntpServer);
  WiFi.begin(MAIN_SSID, MAIN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("#");
  }
  mqttClient.setServer(server, 1883);
  analogReadResolution(10);  //default is 12. Can be set between 9-12.

  for (int i = 0; i < 6; i++) {
    pinMode(inputPins[i], INPUT);
  }


  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);

  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
  digitalWrite(yellow_led, LOW);
}


int do_ = 0;
int calibrate = 1;

int minValues[N] = { 0, 0, 0, 0, 0, 0 };
int avgValues[N] = { 0, 0, 0, 0, 0, 0 };
int maxValues[N] = { 0, 0, 0, 0, 0, 0 };
int values[N];

void loop(){
  // Start calibrating
  if(calibrate){
    Serial.println("Starting to Calibrate. Put a cloth on the sensors (5 seconds))");
    delay(5000); // Wait 5 seconds to reset all esp32s

    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, HIGH);
    digitalWrite(green_led, LOW);
    
    for(int i = 0; i < 6; i++){
      minValues[i] = analogRead(inputPins[i]);  
    }
    Serial.println("Remove the cloth from the sensors (5 seconds))");
    delay(5000); // 5 seconds to remove the cloth

    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, HIGH);
    
    for(int i = 0; i < 6; i++){
      maxValues[i] = analogRead(inputPins[i]);  
    }

    for(int i = 0; i < 6; i++){
      avgValues[i] = (maxValues[i] + minValues[i]) / 2;  
    }
    calibrate = 0;
    Serial.println("Calibration Stopped");
    delay(1000);
  }

  digitalWrite(red_led, LOW);
  digitalWrite(yellow_led, LOW);
  digitalWrite(green_led, HIGH);
  
  while (!mqttClient.connected()) {
    Serial.println("Connect Loop");
    Serial.println(mqttClient.connected());
    // mqttConnect();
    // clientID, username, password
    mqttClient.connect(username, clientID, password);
  }
  if (do_ == 0) {
    Serial.println("MQTT Connected!");
    do_ += 1;
  }
  for (int i = 0; i < N; i++) {
    Data[i] = analogRead(inputPins[i]);
  }
  String output = "";
  for (int i = 0; i < N; i++) {
    if (Data[i] >= avgValues[i]) {
      output += "0,";
      values[i] = 0;
    } else {
      output += "1,";
      values[i] = 1;
    }
  }

  Serial.println(output);
  String dataString = "field" + String(NODE_NUMBER) + "=" + String(output);
  String topicString = "channels/" + String(writeChannelID) + "/publish";
  mqttClient.publish(topicString.c_str(), dataString.c_str());

  epochTime = getTime();

  int len = sizeof(node1) / sizeof(node1[0]);
  String dataStr = "{";
  for (int i = 0; i < len - 1; i++)
    dataStr += ("\\\"" + node1[i] + "\\\":" + String(values[i]) + ", ");
  dataStr += ("\\\"" + node1[len - 1] + "\\\":" + String(values[len - 1]));
  dataStr += "}";
  Serial.println(dataStr);

  String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;

  Serial.println(server);
  String url = server + String() + OM2M_AE + "/" + NODE + "/" + OM2M_DATA_CONT + "/";
  Serial.println(url);
  http.begin(url);
  http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  http.addHeader("Content-Type", "application/json;ty=4");

  String req_data = String() + "{\"m2m:cin\": {"
                    + "\"con\": \"" + dataStr + "\","
                    + "\"lbl\": \"" + NODE + "\","
                    + "\"rn\": \"" + "cin_" + String(epochTime) + "\","
                    + "\"cnf\": \"text\""
                    + "}}";
  Serial.println(req_data);
  int code = http.POST(req_data);
  http.end();
  Serial.println(code);

  delay(2000);
}
