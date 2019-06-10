#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Adafruit_Sensor.h>
#include <EEPROM.h> //0:temp,4:pres,8:altitude, 12:flag, 15:swst
#include <MPU6050_tockn.h>
#include <farmerkeith_BMP280.h>


MPU6050 mpu6050(Wire);
bmp280 bmp0;


#define A 0         //Analog pin 0

tmElements_t tm;

//Hall Effect sensor
volatile byte half_revolutions;
unsigned int rpm_n;

unsigned long timeold;

//GENERAL VARIABLES
int flag;             //reboot check -- flag=0xFF:inicio normal,flag=1:se reinicio 
float team_id = 3666;
float mission_t;
float packet_cnt;               //FALTA!!
float alt_i, alt_o;
float offset;            
float alt_open;
float pressure;
float temp;
float voltage;
float gps_th, gps_tm, gps_ts;
float gps_lat, gps_long, gps_sats, gps_alt;
float pitch, roll;
float rpm;
float swst; //software state:
                      //0.0: BOOT
                      //1.0: LAUNCH_DETECT
                      //2.0: DESCENT
                      //3.0: RELEASED
                      //4.0: LANDED
int match_cnt = 0;    //match count
const int buzzer = 7;
int calibrate = 0;
uint8_t payload[86];
float variables[16];
uint8_t comma;


//MPU variables
float gyroXoffset, gyroYoffset, gyroZoffset;


typedef union{
  float number;
  uint8_t bytes[4];
}FLOATUNION_t;


void bmp();
void comp_alt();
void readingTime();
bool getTime(const char *str);
void magnet_detect();
void voltageSensor();
void imuGyro();
void descent();
void imp();

void setup() {
  flag = EEPROM.read(0);    //if flag=0xFF-> inicio normal
                            //if flag=1   -> se reinicio
                            
  alt_open = 450;         //PARAMETERS TO CHANGE
  offset = 1179.0;
  
                            
  if(flag != 1){  //Inicio normal

    bool parse=false;
    bool config=false;
    
    mission_t = 0.0;
    team_id = 3666;
    packet_cnt = 0.0;
    alt_i = 0.0;
    alt_o = 0.0;
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
  }else{      //flag == 1
    flag = 1;
    alt_o = 0.0;
    mission_t = EEPROM.read(6);   
    packet_cnt = EEPROM.read(10);  
    alt_i = EEPROM.read(14);  
    pressure = EEPROM.read(18); 
    temp = EEPROM.read(22);  
    voltage = EEPROM.read(26); 
    gps_th = EEPROM.read(30);  
    gps_lat = EEPROM.read(34);
    gps_long = EEPROM.read(38);  
    gps_sats = EEPROM.read(46);
    pitch = EEPROM.read(50);  
    roll = EEPROM.read(54);  
    rpm = EEPROM.read(58);  //blade spin rate
    swst = EEPROM.read(62);  //software state
    gps_tm = EEPROM.read(66);
    gps_ts = EEPROM.read(70);
    gyroXoffset = EEPROM.read(74); 
    gyroYoffset = EEPROM.read(78);
    gyroZoffset = EEPROM.read(82);
  }


  

  //Send Data
  Wire.begin();
  Serial.begin(9600);


  //MPU
  mpu6050.begin();
  //Antes de cada prueba limpiar EEPROM
  if(flag != 1){ //Inicio Normal
      mpu6050.calcGyroOffsets(true);
      gyroXoffset = mpu6050.getGyroXoffset(); 
      gyroYoffset = mpu6050.getGyroYoffset();
      gyroZoffset = mpu6050.getGyroZoffset();
      EEPROM.write(74, gyroXoffset);
      EEPROM.write(78, gyroYoffset);
      EEPROM.write(82, gyroZoffset);
  }
  else{
     mpu6050.setGyroOffsets(EEPROM.read(74), EEPROM.read(78), EEPROM.read(82));
  }



  //BMP
  bmp0.begin();
  
  

  //Hall Effect sensor
  attachInterrupt(digitalPinToInterrupt(21), magnet_detect, RISING);//Initialize the intterrupt pin (Arduino digital pin 2)
  half_revolutions = 0;
  timeold = 0;
}

void loop() {
    if(swst == 0.0){          //BOOT Mode
        comp_alt();
    }else if(swst == 1.0){    //ASCEND Mode

       // get the time the compiler was run
        if (getTime(__TIME__)) {
           parse = true;
           // and configure the RTC with this info
           if (RTC.write(tm)) {
                config = true;
            }
        }


        EEPROM.update(6,mission_t);
        comp_alt();
    }else if(swst == 2.0){    //DESCENT Mode
        descent();            //450 m ??
    }else if(swst  == 3.0){   //RELEASED Mode => start transmission
        bmp();   //temp, pressure and altitude readings
        voltageSensor();
        imuGyro();
        readingTime();
                
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
        EEPROM.update(30,gps_t);
        EEPROM.update(34,gps_lat);
        EEPROM.update(38,gps_long);
        EEPROM.update(42,gps_alt);
        EEPROM.update(46,gps_sats);
        EEPROM.update(50,pitch);
        EEPROM.update(54,roll);
        EEPROM.update(58,rpm);
        EEPROM.update(62,swst);

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
  float pressure_temp;
  bool detect_alt = false;
  while(!detect_alt){
    pressure_temp = bmp0.readPressure();
    alt_i = bmp0.calcAltitude(pressure_temp) - offset;
    if(alt_i <= alt_open){
      swst = 3.0;
      detect_alt = true;
    }
  }
}

void just_landed(){
  float pressure_temp;
  pressure_temp = bmp0.readPressure();
  alt_i = bmp0.calcAltitude(pressure_temp) - offset;
  delay(1000);
  alt_o = bmp0.calcAltitude(pressure_temp) - offset;
  if((alt_o - alt_i) < 1.1){
    swst = 4.0;
  }
}

void comp_alt(){
  bool detect = false;
  float rest;
  float pressure_temp;
  int asc = 0;
  int desc = 0;
  while(!detect){
    alt_i = alt_o;
    delay(70);
    pressure_temp = bmp0.readPressure();
    alt_o = bmp0.calcAltitude(pressure_temp) - offset;  //adjust to local forcase
    //Serial.print(alt_i);
    //Serial.print(", ");
    //Serial.print(alt_o);
    //Serial.print("\n");
    rest = alt_o - alt_i;
    if(rest < 0.0){
      desc++;      
    }else{
      asc++;
    }
    if((asc >= 5) && (alt_i > 10.0)){
      swst = 1.0;
      detect = true;        //UP
    }else if(desc >= 5){
      swst = 2.0;
      detect = true;        //DOWN
    }
  }
}

void bmp(){
  //Temperature, pressure, approximate altitude
  temp = bmp0.readTemperature();
  pressure = bmp0.readPressure();
  alt_i = alt_o;                                //vieja
  delay(70);
  alt_o = bmp0.calcAltitude(pressure) - offset;   //nueva
  //Serial.println(temperature);
  //Serial.print(", ");
  //Serial.print(pressure);
  //Serial.print(", ");
  //Serial.print(altitude1); // this should be adjusted to your local forcase
}


void readingTime(){
    RTC.read(tm);
    mission_t = ((60*tm.Minute)+tm.Second); //Mission time in seconds
}


bool getTime(const char *str){
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = 1;
  tm.Minute = 0;
  tm.Second = 0;
  return true;
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
  float x;
  float y;
  float z;

  mpu6050.update();

  x = mpu6050.getAngleX();
  y = mpu6050.getAngleY();
  z = mpu6050.getAngleZ();
  
  pitch = 5;
  roll = 5;   
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
