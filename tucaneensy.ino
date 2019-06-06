#include <Wire.h>
//#include <DS3231.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <EEPROM.h> //0:temp,4:pres,8:altitude, 12:flag, 15:swst
#include<MPU6050.h>


MPU6050 mpu;

#define BMP_SCK 19  //SCL
#define BMP_MISO 18 //SDA
#define A 0         //Analog pin 0

Adafruit_BMP280 bme; // I2C

tmElements_t tm;

//Hall Effect sensor
volatile byte half_revolutions;
unsigned int rpm_n;
float rpm;
unsigned long timeold;
//MPU IMU
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
float x;
float y;
float z;

//GENERAL VARIABLES
int flag;             //reboot check --flag=0xFF:inicio normal,flag=1:se reinicio 
float team_id = 3666;
float mission_t;
float packet_cnt;               //FALTA!!
float alt_i, alt_o;
float pressure;
float temp;
float voltage;
float gps_t;
float gps_lat, gps_long, gps_sats, gps_alt;
float pitch, roll;
float swst; //software state:
                      //0.0: BOOT
                      //1.0: LAUNCH_DETECT
                      //2.0: DESCENT
                      //3.0: RELEASED
                      //4.0: LANDED
int match_cnt = 0;    //match count
//float data[16];  //data to save in EEPROM.
const int buzzer = 7;
int calibrate = 0;
uint8_t payload[86];
float variables[16];
uint8_t comma;

typedef union{
  float number;
  uint8_t bytes[4];
}FLOATUNION_t;


void bmp280();
void comp_alt();
void writeEEPROM();
void readEEPROM();
void updateEEPROM();
void imp();

bool setTime();
void magnet_detect();
void settingTime();
void voltageSensor();
void imuGyro();
void descent();


void setup() {
  flag = EEPROM.read(0);    //if flag=0xFF-> inicio normal
                            //if flag=1   -> se reinicio
  if(flag != 1){
    mission_t = 0.0;
    team_id = 3666;
    packet_cnt = 0.0;
    alt_i = 0.0;
    pressure = 0.0;
    temp = 0.0;
    voltage = 0.0;
    gps_t = 0.0;
    gps_long = 0.0;
    gps_lat = 0.0;
    gps_sats = 0.0;
    gps_alt = 0.0;
    pitch = 0.0;
    roll = 0.0;
    rpm = 0.0;
    swst = 0.0;
    flag = 1;    
    calibrate = 0;
    EEPROM.write(0,flag);
    EEPROM.write(2,team_id);
  }else{
    flag = 1;
    alt_o = 0.0;
    mission_t = EEPROM.read(6);   
    packet_cnt = EEPROM.read(10);  
    alt_i = EEPROM.read(14);  
    pressure = EEPROM.read(18); 
    temp = EEPROM.read(22);  
    voltage = EEPROM.read(26); 
    gps_lat = EEPROM.read(30);
    gps_long = EEPROM.read(34);  
    gps_sats = EEPROM.read(38);
    pitch = EEPROM.read(46);  
    roll = EEPROM.read(50);  
    rpm = EEPROM.read(54);  //blade spin rate
    swst = EEPROM.read(58);  //software state
    gps_t = EEPROM.read(62); 
  }
  Wire.begin();
  //MPU IMU
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  mpu.setXGyroOffset(8);
  mpu.setYGyroOffset(113);
  mpu.setZGyroOffset(54);
  mpu.setXAccelOffset(-2026);
  mpu.setYAccelOffset(-2000);
  mpu.setZAccelOffset(1850);
  //Hall Effect sensor
  attachInterrupt(digitalPinToInterrupt(21), magnet_detect, RISING);//Initialize the intterrupt pin (Arduino digital pin 2)
  half_revolutions = 0;
  timeold = 0;
  //BMP280
  if (!bme.begin()) {
    Serial.println("BMP280 sensor not found, check wiring!");
    while (!Serial);
  }
  Serial.begin(115200);
}

void loop() {
    if(swst == 0.0){          //BOOT Mode
        comp_alt();
    }else if(swst == 1.0){    //ASCEND Mode
        setTime();
        EEPROM.update(6,mission_t);
        comp_alt();
    }else if(swst == 2.0){    //DESCENT Mode
        descent();            //450 m ??
    }else if(swst  == 3.0){   //RELEASED Mode => start transmission
        bmp280();   //temp, pressure and altitude readings
        voltageSensor();
        imuGyro();
        readTime();
        if (half_revolutions >= 3) {
            rpm_n = (60000*half_revolutions)/(millis() - timeold);
            timeold = millis();
            half_revolutions = 0;
            rpm = rpm_n;
            Serial.println(rpm);
        }

        //SPACE FOR GPS FUNCTION
        
        //Updating values to save in EEPROM
        
        EEPROM.update(6,mission_t);
        EEPROM.update(10,packet_cnt);
        EEPROM.update(14,alt_i);
        EEPROM.update(18,pressure);
        EEPROM.update(22,temp);
        EEPROM.update(26,voltage);
        EEPROM.update(30,gps_lat);
        EEPROM.update(34,gps_long);
        EEPROM.update(38,gps_alt);
        EEPROM.update(42,gps_sats);
        EEPROM.update(46,pitch);
        EEPROM.update(50,roll);
        EEPROM.update(54,rpm);
        EEPROM.update(58,swst);
        EEPROM.update(62,gps_t);
        //Convert data to sent through XBees
        imp();
        for(int i = 0; i < 86; ++i){
          Serial.write(payload[i]);
          //delay(10);
        }
        packet_cnt = packet_cnt + 1.0;
        just_landed();      //checking if landed
    }else if(swst == 4.0){
        tone(buzzer, 1000);
        delay(1050);
        noTone(buzzer);
        delay(500);
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
      swst = 3.0;            //RELEASED Mode
    }
  }
}

void just_landed(){
    alt_i = bme.readAltitude(1013.25);
    delay(10);
    alt_o = bme.readAltitude(1013.25);
    if((alt_o - alt_i) == 0){
      swst = 4.0;
    }
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
      swst = 1.0;
    }else if(desc >= 7){
      detect = true;        //DOWN
      swst = 2.0;
    }
  }
}

void bmp280(){
  //Temperature, pressure, approximate altitude
  temp = bme.readTemperature();
  pressure = bme.readPressure() * 100;
  alt_i = alt_o;                       //vieja
  alt_o = bme.readAltitude(1013.25);   //nueva
  //Serial.print(temperature);
  //Serial.print(", ");
  //Serial.print(pressure);
  //Serial.print(", ");
  //Serial.print(altitude1); // this should be adjusted to your local forcase
  //Serial.print("\n");

}

bool setTime(){
    int Min = 00;
    int Sec = 00;
    tm.Minute = Min;
    tm.Second = Sec;
    return true;
}

void readTime(){
    RTC.read(tm);
    mission_t = (tm.Minute*60)+tm.Second;
}

void magnet_detect()//This function is called whenever a magnet/interrupt is detected by the arduino
 {
   half_revolutions++;
   //Serial.println("detect");
}

void voltageSensor(){
  //read the input on analog pin 0:
  int sensorValue = analogRead(A);
  //Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (3.3 / 1023.0);
  //print out the value you read:
  //Serial.println(voltage);
}


void imuGyro(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);
  int zAng = map(AcZ,minVal,maxVal,-90,90);
  x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  y = 360-y;
  if (x > 180){
    x = x-360;
  }
  if (y > 180){
    y = y-360;
  }
  roll = x;
  pitch = y;
  //AngleX (Roll), AngleY (Pitch), AngleZ
  //Serial.println(x);
  //Serial.print(", ");
  //Serial.print(y);
  //Serial.print(", ");
  //Serial.print(z);
  //Serial.print("\n");
}

void imp(){
  
  FLOATUNION_t myfloat;
  int cont = 0;

  variables[0] = team_id;
  variables[1] = mission_t;
  variables[2] = packet_cnt;
  variables[3] = alt_i;
  variables[4] = pressure;
  variables[5] = temp;
  variables[6] = voltage;
  variables[7] = gps_t;
  variables[8] = gps_lat;
  variables[9] = gps_long;
  variables[10] = gps_alt;
  variables[11] = gps_sats;
  variables[12] = pitch;
  variables[13] = roll;
  variables[14] = rpm;
  variables[15] = swst;
  comma = ',';

  int i=0;
  int j=0;
  for(i = 0; i < 18; i++){
    myfloat.number = variables[i];
    for(j = 0; j < 4; j++){
      payload[cont] = myfloat.bytes[j];
      ++cont;
    }
    if(i != 7 && i != 8 && i != 17){
      payload[cont] = comma;
      ++cont;
    }
  }
}
