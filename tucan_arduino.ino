#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <EEPROM.h> //0:temp,4:pres,8:altitude, 12:flag, 15:swst
#include <Servo.h>

#define BMP_MOSI 5 //SDA
#define BMP_SCK 6   //SCL

Servo servin;
Adafruit_BMP280 bme; // I2C
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

int pos = 0;
float temp, pres, alti1, alti2;
int flag;
int desc_m = 7;
int count = 0;
int y=5;
int swst; //software state:
                      //0: BOOT
                      //1: LAUNCH_DETECT
                      //2: DESCENT
                      //3: RELEASE
                      //4: LANDED
const int buzzer = 7;
int asc, desc;
void bmp280();
void comp_alti();
void writeEEPROM();
void readEEPROM();
void updateEEPROM();

void setup() {
  flag = EEPROM.read(12);  //if flag=0xFF-> inicio normal
                           //if flag=1 -> launch activado = se reinicio 
  if(flag != 1){    //inicio normal
    //Initializing data as 0
    temp = 0;
    pres = 0;
    alti1 = 0;
    alti2 = 0;
    swst = 0;
    writeEEPROM();
    flag = 1;
    asc = 0;
    desc = 0;
    EEPROM.write(12, flag);
  }else{           //if flag=1 -> launch activado = se reinicio 
    readEEPROM();
    alti2 = 0;
    asc = 0;
    desc = 0;
  }
  Serial.begin(9600);
  if (!bme.begin()) {
    Serial.println("BMP280 sensor not found, check wiring!");
    while (1);
  }
  
  servin.attach(13);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  if(swst == 0){        //BOOT Mode
    comp_alti();
    updateEEPROM();
  }else if(swst == 1){        //LAUNCH_DETECTED Mode
    comp_alti();
    updateEEPROM();
  }else if(swst == 2){        //DESCENT Mode
    count = 0;
    bool detect = false;
    while(!detect){
      bmp280();
      if(alti1 <= 450.0){
        count++;
      }else{
        count = 0;
      }
      if(count >= y){
        swst = 3;
      }
    } 
  }else if(swst == 3){        //RELEASE Mode
    //Movimiento del servo
    pos = servin.read();
    pos +=70;
    servin.write(pos);
    swst = 4;
  }else if(swst == 4){        //CONT_DESCENT
    alti2 = bme.readAltitude(1013.25); 
    delay(1000);
    alti1 = bme.readAltitude(1013.25); 
    if((alti2 - alti1) == 0){ //LANDED!!!
      swst == 5;
    }
  }else if(swst == 5){        //LANDED Mode
    tone(buzzer, 1000);
    delay(1000);
    noTone(buzzer);
    delay(500);    
  }
  delay(500);
}


void bmp280(){
  //Temperature, pressure, approximate altitude
  temp = bme.readTemperature();
  pres = bme.readPressure();
  alti2 = alti1;                       //vieja
  alti1 = bme.readAltitude(1013.25);   //nueva
  Serial.print(temp);
  Serial.println(", ");
  Serial.print(pres);
  Serial.println(", ");
  Serial.print(alti1); // this should be adjusted to your local forcase
  Serial.println("\n");
  EEPROM.update(8, alti1);
}

void comp_alti(){
   bool detect = false;
   while(!detect){
    bmp280();
    if((alti2 - alti1) > 0 && abs(alti2-alti1) > desc_m){
      desc++;   
    }else{
      asc++;
    }
    if(asc >= 7){
      detect = true;
      swst = 1;             //LAUNCH_DETECT
      asc = 0;
    }else if(desc >= 7){
      detect = true;
      swst = 2;             //DESCENT INITIATED
      desc = 0;
    }
  }
}

void writeEEPROM(){
  EEPROM.write(8, alti1);
  EEPROM.write(15, swst);
}
void readEEPROM(){
  alti1 = EEPROM.read(8);
  swst = EEPROM.read(15);
}
void updateEEPROM(){
  EEPROM.update(8,alti1);
}
