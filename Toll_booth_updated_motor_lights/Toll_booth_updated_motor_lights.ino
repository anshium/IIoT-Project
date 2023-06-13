#include <Servo.h>
#include <Wire.h>
#include "U8glib.h"

////////////// I2C code /////////////////////////
/////////////////////////////////////////////////

// #define SLAVE_ADDRESS 8
// char receivedData[32];


/////////////////////////////////////////////////
/////////////////////////////////////////////////



int spot = 1;

U8GLIB_SH1106_128X64 oled(U8G_I2C_OPT_NONE);

const int servoPin = 7;

Servo servoMotor;

const int trigPin = 11;
const int echoPin = 10;

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

int lb = 2, ub = 40;


void setup() {

  // Wire.begin(0x04); // Set Arduino's I2C address to 0x04
  // Wire.onReceive(receiveEvent);

  Wire.begin(SLAVE_ADDRESS); // Initialize I2C communication with the slave address
  Wire.onReceive(receiveEvent); // Set up the event handler for receiving data



  Serial.begin(9600);
  servoMotor.attach(servoPin);
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  pinMode(3, OUTPUT);
  // digitalWrite(9, LOW);
  digitalWrite(3,HIGH);
  pinMode(4,OUTPUT);
  pinMode(2,OUTPUT);
  servoMotor.write(0);
}

void interchange(int time){
  for(int i = 0; i < time / 200; i++){
    digitalWrite(2, HIGH);
    digitalWrite(4, LOW);
    delay(100);
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
    delay(100);
  }
}

void loop(void) {
  digitalWrite(3,HIGH);
  //digitalWrite(9, LOW);
  
  // Move the servo to the starting position (0 degrees)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;

  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;

  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  oled.firstPage();
    do {
      page4();
    } while (oled.nextPage());
    delay(1000);
  // interchange(1000);
  if (distanceCm < 8) {

    Serial.println(distanceCm);
    digitalWrite(9, HIGH);

    oled.firstPage();
    do {
      page1();
    } while (oled.nextPage());
    // delay(1000);
    interchange(1000);
    oled.firstPage();
    do {
      page2();
    } while (oled.nextPage());
    // delay(1000);
    interchange(1000);
    oled.firstPage();
    do {
      page3();
    } while (oled.nextPage());
    servoMotor.write(90);
    digitalWrite(2,HIGH);
    digitalWrite(4,LOW);
    delay(3000);
    spot = (rand() % (ub - lb + 1)) + lb;
    servoMotor.write(0);
  }
  //servoMotor.write(0);
  
  digitalWrite(4,HIGH);
  digitalWrite(2,LOW);
  delay(100);
}
  

void page1(void) {
  oled.setFont(u8g_font_profont12);
  oled.setPrintPos(0, 10);
  oled.print("Welcome");
}

void page2(void) {
  oled.setFont(u8g_font_profont12);
  oled.setPrintPos(0, 10);
  oled.println("Your Parking Spot");
}

void page3(void) {
  oled.setFont(u8g_font_profont12);
  oled.setPrintPos(0, 10);
  oled.println("is A");
  oled.print(spot);
}

void page4(void) {
  oled.setFont(u8g_font_profont12);
  oled.setPrintPos(0, 10);
  oled.println("");
}

