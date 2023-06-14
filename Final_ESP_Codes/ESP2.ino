#include <ThingSpeak.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define CSE_IP "192.168.57.64"

int selectPins[3] = { 25, 13, 26 };
int readPins[2] = { 32, 33 };
String out = "";

#define MAIN_SSID "Galaxy M31FCCB"
#define MAIN_PASS "pvub5546"

#define CSE_PORT 5089
#define OM2M_ORGIN "admin:admin"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "SmartParkingSystem"
#define NODE "SubStation-2"
#define NODE_NUMBER 2
#define OM2M_DATA_CONT "Data"

#define N 16

#define username "FwYNDA4VCzkuMigUNjwXMCM"
#define clientID "FwYNDA4VCzkuMigUNjwXMCM"
#define password "ppieJfYd1hSevcdgpXYrDP2H"

String labels[N] = { "i32", "i1", "i4", "B6", "i5", "i28", "i31", "i27",
                     "i29", "i7", "i6", "", "", "i3", "", "" };

int Data[N] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

const char server[] = "mqtt3.thingspeak.com";
int writeChannelID = 2167101;
char writeAPIKey[] = "N2DPU8YFUT8TZ4AV";

const char* ntpServer = "pool.ntp.org";
unsigned long epochTime;

WiFiClient myClient;
HTTPClient http;
PubSubClient mqttClient(server, 1883, myClient);

int red_led = 12;
int yellow_led = 14;
int green_led = 27;

int mode_ = 0;

int pb = 34;

void setup() {
  Serial.begin(9600);
  WiFi.begin(MAIN_SSID, MAIN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(100);
  }
  Serial.println("Connected.");
  mqttClient.setServer(server, 1883);

  analogReadResolution(10);  //default is 12. Can be set between 9-12.
  for (int i = 0; i < 3; i++) {
    pinMode(selectPins[i], OUTPUT);
  }
  for (int i = 0; i < 2; i++) {
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
int calibrate = 1;

int minValues[N] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int avgValues[N] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int maxValues[N] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int values[N];

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
    return (0);

  time(&now);
  return now;
}

void loop() {
  if (calibrate) {
    Serial.println("Starting to Calibrate. Put a cloth on the sensors (5 seconds))");
    delay(5000);  // Wait 5 seconds to reset all esp32s

    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, HIGH);
    digitalWrite(green_led, LOW);

    for (int mux = 0; mux < 2; mux++) {
      int readPin = readPins[mux];
      for (int j = 0; j < 8; j++) {
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
    for (int i = 0; i < 16; i++) {
      out += String(minValues[i]) + ",";
    }
    Serial.println(out);
    Serial.println("Remove the cloth from the sensors (5 seconds))");
    delay(5000);  // 5 seconds to remove the cloth

    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, HIGH);

    for (int mux = 0; mux < 2; mux++) {
      int readPin = readPins[mux];
      for (int j = 0; j < 8; j++) {
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
    for (int i = 0; i < 16; i++) {
      out += String(maxValues[i]) + ",";
    }
    Serial.println(out);

    for (int i = 0; i < 16; i++) {
      avgValues[i] = (maxValues[i] + minValues[i]) / 2;
    }

    out = "";
    for (int i = 0; i < 16; i++) {
      out += avgValues[i] + ",";
    }
    calibrate = 0;
    Serial.println("Calibration Stopped");
    delay(1000);
  }
  while (!mqttClient.connected()) {
    Serial.println("Connect Loop");
    Serial.println(mqttClient.connected());
    // mqttConnect();
    // clientID, username, password
    mqttClient.connect(clientID, username, password);
  }
  if (do_ == 0) {
    Serial.println("MQTT Connected!");
    do_ += 1;
  }

  for (int mux = 0; mux < 2; mux++) {
    int readPin = readPins[mux];
    for (int j = 0; j < 8; j++) {
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
  for (int i = 0; i < 16; i++) {
    if (Data[i] < avgValues[i]) {
      sensorData += "1,";
      values[i] = 1;
    } else {
      sensorData += "0,";
      values[i] = 0;
    }
  }
  Serial.println(sensorData);
  String dataString = "field" + String(NODE_NUMBER) + "=" + sensorData;
  String topicString = "channels/" + String(writeChannelID) + "/publish";

  mqttClient.publish(topicString.c_str(), dataString.c_str());

  epochTime = getTime();

  int len = N;
  String dataStr = "{";
  for (int i = 0; i < len - 1; i++)
    dataStr += ("\\\"" + labels[i] + "\\\":" + String(values[i]) + ", ");
  dataStr += ("\\\"" + labels[len - 1] + "\\\":" + String(values[len - 1]));
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
                    + "\"cnf\": \"text\""
                    + "}}";
  Serial.println(req_data);
  int code = http.POST(req_data);
  http.end();
  Serial.println(code);

  delay(80);
}
