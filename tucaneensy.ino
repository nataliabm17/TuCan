#include <Wire.h>
//#include <DS3231.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <EEPROM.h> //0:temp,4:pres,8:altitude, 12:flag, 15:swst

#define BMP_SCK 19  //SCL
#define BMP_MISO 18 //SDA
#define A 0         //Analog pin 0

Adafruit_BMP280 bme; // I2C

//RTC DS3231
//DS3231 Clock;
tmElements_t tm;

//Hall Effect sensor
volatile byte half_revolutions;
unsigned int rpm;
unsigned long timeold;
//MPU IMU
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
double x;
double y;
double z;


//GENERAL VARIABLES
int flag;             //reboot check --flag=0xFF:inicio normal,flag=1:se reinicio 
int team_id = 3666;
int mission_t;
int packet_cnt;               //FALTA!!
float alt_i, alt_o;
float pressure;
float temperature;
float voltage;
string gps_t;
float gps_lat, gps_long, gps_sats;
float pitch, roll;
int swst; //software state:
                      //0: BOOT
                      //1: LAUNCH_DETECT
                      //2: DESCENT
                      //3: RELEASED
                      //4: LANDED
int match_cnt = 0;    //match count
//float data[16];  //data to save in EEPROM.
const int buzzer = 7;
int calibrate = 0;
void bmp280();
void comp_alt();
void writeEEPROM();
void readEEPROM();
void updateEEPROM();


bool getTime(const char *str);
void magnet_detect();
void settingTime();
void voltageSensor();
void imuGyro();
void descent();


void setup() {
  flag = EEPROM.read(0);    //if flag=0xFF-> inicio normal
                            //if flag=1   -> se reinicio
  if(flag != 1){
    mission_t = 0;
    team_id = 3666;
    packet_cnt = 0;
    alt_i = 0.0;
    pressure = 0.0;
    temperature = 0.0;
    voltage = 0.0;
    gps_t = "";
    gps_long = 0.0;
    gps_sats = 0.0;
    pitch = 0.0;
    roll = 0.0;
    rpm = 0.0;
    swst = 0;
    flag = 1;    
    calibrate = 0;
    EEPROM.write(0,flag);
    EEPROM.write(2,team_id);
    bool parse = false;
    bool config = false;
    if(getTime(__TIME__)){
        parse = true;
        if(RTC.write(tm)){
            config = true;
        }
    }
  }else{
    flag = 1;
    alt_o = 0;
    EEPROM.read(4,mission_t);
    EEPROM.read(6,packet_cnt);
    EEPROM.read(8,alt_i);
    EEPROM.read(12,pressure);
    EEPROM.read(16,temperature);
    EEPROM.read(20,voltage);
    EEPROM.read(24,gps_long);
    EEPROM.read(28,gps_sats);
    EEPROM.read(32,pitch);
    EEPROM.read(36,roll);
    EEPROM.read(40,rpm);
    EEPROM.read(44,swst);
    EEPROM.read(46,gps_t);
  }
  Wire.begin();
  //MPU IMU
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  //Hall Effect sensor
  attachInterrupt(digitalPinToInterrupt(21), magnet_detect, RISING);//Initialize the intterrupt pin (Arduino digital pin 2)
  half_revolutions = 0;
  timeold = 0;
  //BMP280
  if (!bme.begin()) {
    Serial.println("BMP280 sensor not found, check wiring!");
    while (!Serial);
  }
}

void loop() {
    if(swst == 0){          //BOOT Mode
        comp_alt();
    }else if(swst == 1){    //ASCEND Mode
        getTime();
        EEPROM.update(4,mission_t);
        comp_alt();
    }else if(swst == 2){    //DESCENT Mode
        descent();          //450 m ??
    }else if(swst  == 3){   //RELEASED Mode => start transmission
        bmp280();   //temp, pressure and altitude readings
        voltageSensor();
        imuGyro();
        readTime();
        if (half_revolutions >= 3) {
            rpm = (60000*half_revolutions)/(millis() - timeold);
            timeold = millis();
            half_revolutions = 0;
            Serial.println(rpm);
        }
        EEPROM.update(4,mission_t);
        EEPROM.update(6,packet_cnt);
        EEPROM.update(8,alt_i);
        EEPROM.update(12,pressure);
        EEPROM.update(16,temperature);
        EEPROM.update(20,voltage);
        EEPROM.update(24,gps_long);
        EEPROM.update(28,gps_sats);
        EEPROM.update(32,pitch);
        EEPROM.update(36,roll);
        EEPROM.update(40,rpm);
        EEPROM.update(44,swst);
        EEPROM.update(46,gps_t);
        
        just_landed();      //checking if landed
    }else if(swst == 4){
        tone(buzzer, 1000);
        delay(1050);
        noTone(buzzer,500);
        delay(500);
    }
    
    
    //Hall Effect sensor readings
    if (half_revolutions >= 3) {
      rpm = (60000*half_revolutions)/(millis() - timeold);
      timeold = millis();
      half_revolutions = 0;
      Serial.println(rpm);
    }
    //Voltage sensor readings
    
    delay(2000);
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
      swst = 3;            //RELEASED Mode
    }
  }
}

void just_landed(){
    alt_i = bme.readAltitude(1013.25);
    delay(10);
    alt_o = bme.readAltitude(1013.25);
    if((alt_o - alt_i) == 0){
      swst = 5;
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
      swst = 1;
    }else if(desc >= 7){
      detect = true;        //DOWN
      swst = 2;
    }
  }
}

void bmp280(){
  //Temperature, pressure, approximate altitude
  temperature = bme.readTemperature();
  pressure = bme.readPressure();
  alt_i = alt_o;                       //vieja
  alt_o = bme.readAltitude(1013.25);   //nueva
  //Serial.print(temperature);
  //Serial.print(", ");
  //Serial.print(pressure);
  //Serial.print(", ");
  //Serial.print(altitude1); // this should be adjusted to your local forcase
  Serial.print("\n");

}

bool getTime(){
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
    s
void magnet_detect()//This function is called whenever a magnet/interrupt is detected by the arduino
 {
   half_revolutions++;
   //Serial.println("detect");
}
void voltageSensor(){
  //read the input on analog pin 0:
  int sensorValue = analogRead(A);
  //Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (3.3 / 1023.0);
  //print out the value you read:
  Serial.println(voltage);
}

void settingTime(){
    GetDateStuff(Year, Month, Date, DoW, Hour, Minute, Second);
    Clock.setClockMode(false);
    Clock.setYear(Year);
    Clock.setMonth(Month);
    Clock.setDate(Date);
    Clock.setDoW(DoW);
    Clock.setHour(Hour);
    Clock.setMinute(Minute);
    Clock.setSecond(Second);
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
  z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
  //AngleX, AngleY, AngleZ
  Serial.print("\n");
  Serial.println(x);
  Serial.print(", ");
  Serial.println(y);
  Serial.print(", ");
  Serial.println(z);
  Serial.println("\n");
  delay(400);
}
