#include <FirmataDefines.h>
#include <Boards.h>
#include <FirmataMarshaller.h>
#include <FirmataConstants.h>
#include <Firmata.h>
#include <FirmataParser.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>

Servo serv;
Adafruit_BMP280 bme;  //I2C

int swst;

float offset;           
float alt_open;
const int buzzer = 4;
float alt_i, alt_o;

void comp_alt();
void descent();

//CONTAINER 1
int serv_opened_pos = 165;
int serv_closed_pos = 95;
//CONTAINER 2
//int serv_opened_pos = 160;
//int serv_closed_pos = 90;

void setup() {  
  offset = 1379.0;              //PARAMETERS TO CHANGE
  alt_open = 203.0;
  
  pinMode(buzzer, OUTPUT);
  serv.attach(9);
  Serial.begin(9600);
  if (!bme.begin()) {
    Serial.println("BMP280 sensor not found, check wiring!");
    while (1);
  }
  alt_i = 0.0;
  alt_o = 0.0;
  swst = 0;
  
  /**************/    //ON SOUND
  tone(buzzer, 1000);
  delay(200);
  noTone(buzzer);
  delay(200);
  tone(buzzer, 1000);
  delay(200);
  noTone(buzzer);
  delay(200);
  
  /**************/              //OPEN SERVO TO PLACE AUTOGYRO
  serv.write(serv_opened_pos);    
  delay(5000);

  /**************/              //PRECLOSE SOUND 
  tone(buzzer, 1000);
  delay(200);
  noTone(buzzer);
  delay(900);

  /**************/

  serv.write(serv_closed_pos);  //CLOSE SERVO WITH AUTOGYRO PLACED

}



void loop() {
  //Serial.println(bme.readAltitude(1013.25) - offset);
  if (swst == 0) {      //BOOT Mode
    comp_alt();
    Serial.println("Fin primera comparacion");
  } else if (swst == 1) { //UP Mode
    comp_alt();
    Serial.println("Fin comparacion UP");
  } else if (swst == 2) { //DOWN Mode
    descent();
    Serial.println("Fin comparacion DOWN");
  } else if (swst == 3) { //SERVO_TIME
    //    int pos = serv.read();
    //    pos += 70;
    serv.write(serv_opened_pos);
    delay(5000);
    swst = 4;
  } else if (swst == 4) { //CONTINUING_DESCENT Mode
    alt_i = bme.readAltitude(1013.25) - offset;
    delay(90);
    alt_o = bme.readAltitude(1013.25) - offset;
    if ((alt_o - alt_i) < 1.1) {
      swst = 5;
    }
  } else if (swst == 5) { //JUST_LANDED Mode
    tone(buzzer, 4000);
    delay(1000);
    noTone(buzzer);
    delay(50);
  }
  delay(30);

}

/****************************************************************/
/****************************************************************/
/****************************************************************/

void comp_alt() {
  bool detect = false;
  float rest;
  int asc = 0;
  int desc = 0;
  int desc_m = 7;
  while (!detect) {
    alt_i = alt_o;
    delay(100);
    alt_o = bme.readAltitude(1013.25) - offset;  //adjust to local forcase
    Serial.print(alt_i);
    Serial.print(", ");
    Serial.print(alt_o);
    Serial.print("\n");
    rest = alt_o - alt_i;
    if (rest < 0.0) {
      desc++;
      Serial.print("Desciende\n");
    } else {
      asc++;
      Serial.print("Asciende\n");
    }
    if ((asc >= 5) && (alt_i > 10.0)) {
      Serial.print("UP*****************\n");
      swst = 1;
      detect = true;        //UP
    } else if ((desc >= 5) && (swst == 1)) {
      swst = 2;
      Serial.print("DOWN--------------------\n");
      detect = true;        //DOWN
    }
  }
}

void descent() {
  int count = 0;
  bool detect_alt = false;
  Serial.println("Entra a descent");
  while (!detect_alt) {
    alt_i = bme.readAltitude(1013.25) - offset;
    if (alt_i <= alt_open) {
      swst = 3;
      Serial.println("Servo Time");
      detect_alt = true;
    }
  }
}
