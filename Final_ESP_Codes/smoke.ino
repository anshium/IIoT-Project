#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

#define DHTPIN 4       // Pin connected to the DHT sensor
#define DHTTYPE DHT11  // Change to DHT22 if you're using that sensor
#define BUZZER_PIN 5

DHT dht(DHTPIN, DHTTYPE);

#define MAIN_SSID "Galaxy M31FCCB"
#define MAIN_PASS "pvub5546"
#define CSE_IP "192.168.57.64"
#define CSE_PORT 5089
#define OM2M_ORGIN "admin:admin"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "SmartParkingSystem"
#define OM2M_DATA_CONT "SmokeDetection/Data"
#define INTERVAL 1000L

long randNumber;
long int prev_millis = 0;
unsigned long epochTime;

int i = 0;

HTTPClient http;

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  WiFi.begin(MAIN_SSID, MAIN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("#");
  }

  dht.begin();
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {

  float temperature = 0;
  float humidity = 0;

  digitalWrite(5, HIGH);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  Serial.println(temperature);

  if(temperature > 70)
  {
    Serial.println("Alert");
    digitalWrite(5, LOW);
    delay(1000);
    digitalWrite(5, HIGH);
  }

  if (millis() - prev_millis >= INTERVAL) {
    epochTime = getTime();
    String data;
    String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
    Serial.println(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT + "/");

    http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT + "/");

    http.addHeader("X-M2M-Origin", OM2M_ORGIN);
    http.addHeader("Content-Type", "application/json;ty=4");
    http.addHeader("Content-Length", "100");

    String req_data = String() + "{\"m2m:cin\": {"

                      + "\"con\": \"" + temperature + "," + humidity + "\","

                      + "\"lbl\": \"" + "V1.0.0" + "\","

                      + "\"cnf\": \"text\""

                      + "}}";

    Serial.println(req_data);
    int code = http.POST(req_data);
    http.end();
    Serial.println(code);
    prev_millis = millis();
  }
  delay(500);
  i++;
}
