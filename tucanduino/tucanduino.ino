#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>

#define BMP_MOSI 2     //SDA
#define BMP_SCK 3      //SCL

Servo serv;
Adafruit_BMP280 bme;  //I2C

int swst;
const int buzzer = 7;
float alt_i, alt_o;

void comp_alt();
void descent();

void setup() {
  Serial.begin(9600);
  if (!bme.begin()) {
    Serial.println("BMP280 sensor not found, check wiring!");
    while (1);
  }
  alt_i = 0.0;
  alt_o = 0.0;
  swst = 0;
  serv.attach(13);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  if(swst == 0){        //BOOT Mode
    comp_alt();
  }else if(swst == 1){  //UP Mode
    comp_alt();
  }else if(swst == 2){  //DOWN Mode
    descent();
  }else if(swst == 3){  //SERVO_TIME Mode
    int pos = serv.read();
    pos += 70;
    serv.write(pos);
    swst = 4;
  }else if(swst == 4){  //CONTINUING_DESCENT Mode
    alt_i = bme.readAltitude(1013.25);
    delay(10);
    alt_o = bme.readAltitude(1013.25);
    if((alt_o - alt_i) == 0){
      swst = 5;
    }
  }else if(swst == 5){  //JUST_LANDED Mode
    tone(buzzer, 1000);
    delay(1050);
    noTone(buzzer);
    delay(500);
  }
  delay(300);
}


void comp_alt(){
  bool detect = false;
  float rest;
  int asc = 0;
  int desc = 0;
  int desc_m = 7;
  while(!detect){
    alt_i = alt_o;
    alt_o = bme.readAltitude(1013.25);  //adjust to local forcase
    //Serial.print(alt_i);
    //Serial.println(", ");
    //Serial.print(alt_o);
    //Serial.println("\n");
    rest = alt_o - alt_i;
    if((rest > 0) && (rest > desc_m)){
      desc++;
    }else{
      asc++;
    }
    if(asc >= 7){
      detect = true;        //UP
      swst = 1;
    }else if(desc >= 7){
      detect = true;        //DOWN
      swst = 2;
    }
  }
}

void descent(){
  int count = 0;
  bool detect_alt = false;
  while(!detect_alt){
    alt_i = bme.readAltitude(1013.25);
    if(alt_i <= 450.0){
      count++;
    }else{
      count = 0;
    }
    if(count >= 5){
      swst = 3;            //SERVO Time
    }
  }
}
