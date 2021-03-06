#include <DS1307RTC.h>
#include <TimeLib.h>
#include <Adafruit_GPS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DS1307RTC.h>
#include <EEPROM.h> 
#include <MPU6050_tockn.h>
#include <farmerkeith_BMP280.h>


MPU6050 mpu6050(Wire);
bmp280 bmp0;


//---XBEE---
#define Xbee Serial1


//---GPS---
#define GPSSerial Serial3
Adafruit_GPS GPS(&GPSSerial);

uint32_t timer = millis();

#define A 10         //Analog pin 10

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
uint8_t payload[92];
float variables[19];
uint8_t comma;

bool parse;
bool config;


float yellow = 7.2;
float red = 6.0;

int redPin = 3;
int bluePin = 4;
int greenPin = 5;



//MPU variables
float gyroXoffset, gyroYoffset, gyroZoffset;


typedef union{
  float number;
  uint8_t bytes[4];
}FLOATUNION_t;
char c; 

void bmp();
void comp_alt();
void readingTime();
bool getTime(const char *str);
void magnet_detect();
void voltageSensor();
void imuGyro();
void descent();
void imp();
void gps();
void just_landed();
void rgbLED();

void setup() {

  tone(buzzer, 850);
  delay(4050);
  noTone(buzzer);
  delay(500);
  
  flag = EEPROM.read(0);    //if flag=0xFF-> inicio normal
                            //if flag=1   -> se reinicio
                            
  alt_open = 450;         //PARAMETERS TO CHANGE
  offset = 1179.0;

  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  
  
                            
  if(flag != 1){  //Inicio normal

    parse=false;
    config=false;
    
    mission_t = 0.0;
    team_id = 3666;
    packet_cnt = 0.0;
    alt_i = 0.0;
    alt_o = 0.0;
    pressure = 0.0;
    temp = 0.0;
    voltage = 0.0;
    gps_th = 0.0;
    gps_tm = 0.0;
    gps_ts = 0.0;
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
  Serial.begin(9600);
  Xbee.begin(115200);
  GPS.begin(9600);
  Wire.begin();


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
       //---GPS---
       // read data from the GPS in the 'main loop'
       c = GPS.read();
       // if a sentence is received, we can check the checksum, parse it...
       if (GPS.newNMEAreceived()) {
          if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
            return; // we can fail to parse a sentence in which case we should just wait for another
       }    
       // if millis() or timer wraps around, we'll just reset it
       if (timer > millis()) timer = millis();
       // approximately every 1 second
       if (millis() - timer > 1000) { 
          timer = millis(); // reset the timer      
      
        bmp();   //temp, pressure and altitude readings
        voltageSensor();
        rgbLED();
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
        gps();
        //Updating values to save in EEPROM
        EEPROM.update(6,mission_t);
        EEPROM.update(10,packet_cnt);
        EEPROM.update(14,alt_i);
        EEPROM.update(18,pressure);
        EEPROM.update(22,temp);
        EEPROM.update(26,voltage);
        EEPROM.update(30,gps_th);
        EEPROM.update(34,gps_lat);
        EEPROM.update(38,gps_long);
        EEPROM.update(42,gps_alt);
        EEPROM.update(46,gps_sats);
        EEPROM.update(50,pitch);
        EEPROM.update(54,roll);
        EEPROM.update(58,rpm);
        EEPROM.update(62,swst);
        EEPROM.update(66, gps_tm);
        EEPROM.update(70, gps_ts);
        EEPROM.update(74, gyroXoffset); 
        EEPROM.update(78, gyroYoffset);
        EEPROM.update(82, gyroZoffset);

        //Convert data to sent through XBees
        imp();
        for(int i = 0; i < 92; ++i){
          Xbee.write(payload[i]);
          //delay(10);
        }
        packet_cnt = packet_cnt + 1.0;
      }

      
    }else if(swst == 1.0){    //ASCEND Mode
        // get the time the compiler was run
       if (getTime(__TIME__)) {
          parse = true;
          // and configure the RTC with this info
          if (RTC.write(tm)) {
               config = true;
          }
       }
       comp_alt();
       //---GPS---
       // read data from the GPS in the 'main loop'
       c = GPS.read();
       // if a sentence is received, we can check the checksum, parse it...
       if (GPS.newNMEAreceived()) {
          if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
            return; // we can fail to parse a sentence in which case we should just wait for another
       }
       // if millis() or timer wraps around, we'll just reset it
       if (timer > millis()) timer = millis();
       // approximately every 1 second
       if (millis() - timer > 1000) { 
          timer = millis(); // reset the timer      
        bmp();   //temp, pressure and altitude readings
        voltageSensor();
        rgbLED();
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
        gps();
        //Updating values to save in EEPROM
        EEPROM.update(6,mission_t);
        EEPROM.update(10,packet_cnt);
        EEPROM.update(14,alt_i);
        EEPROM.update(18,pressure);
        EEPROM.update(22,temp);
        EEPROM.update(26,voltage);
        EEPROM.update(30,gps_th);
        EEPROM.update(34,gps_lat);
        EEPROM.update(38,gps_long);
        EEPROM.update(42,gps_alt);
        EEPROM.update(46,gps_sats);
        EEPROM.update(50,pitch);
        EEPROM.update(54,roll);
        EEPROM.update(58,rpm);
        EEPROM.update(62,swst);
        EEPROM.update(66, gps_tm);
        EEPROM.update(70, gps_ts);
        EEPROM.update(74, gyroXoffset); 
        EEPROM.update(78, gyroYoffset);
        EEPROM.update(82, gyroZoffset);

        //Convert data to sent through XBees
        imp();
        for(int i = 0; i < 92; ++i){
          Xbee.write(payload[i]);
          //delay(10);
        }
        packet_cnt = packet_cnt + 1.0;
      }
       
    }else if(swst == 2.0){    //DESCENT Mode
        descent();            //450 m ??
        //---GPS---
        // read data from the GPS in the 'main loop'
        c = GPS.read();

        // if a sentence is received, we can check the checksum, parse it...
        if (GPS.newNMEAreceived()) {
           if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
             return; // we can fail to parse a sentence in which case we should just wait for another
        }
                
        // if millis() or timer wraps around, we'll just reset it
        if (timer > millis()) timer = millis();

        // approximately every 1 second
        if (millis() - timer > 1000) { 
           timer = millis(); // reset the timer            
           bmp();   //temp, pressure and altitude readings
           voltageSensor();
           rgbLED();
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
           gps();
           //Updating values to save in EEPROM
           EEPROM.update(6,mission_t);
           EEPROM.update(10,packet_cnt);
           EEPROM.update(14,alt_i);
           EEPROM.update(18,pressure);
           EEPROM.update(22,temp);
           EEPROM.update(26,voltage);
           EEPROM.update(30,gps_th);
           EEPROM.update(34,gps_lat);
           EEPROM.update(38,gps_long);
           EEPROM.update(42,gps_alt);
           EEPROM.update(46,gps_sats);
           EEPROM.update(50,pitch);
           EEPROM.update(54,roll);
           EEPROM.update(58,rpm);
           EEPROM.update(62,swst);
           EEPROM.update(66, gps_tm);
           EEPROM.update(70, gps_ts);
           EEPROM.update(74, gyroXoffset); 
           EEPROM.update(78, gyroYoffset);
           EEPROM.update(82, gyroZoffset);
           //Convert data to sent through XBees
           imp();
           for(int i = 0; i < 92; ++i){
               Xbee.write(payload[i]);
           //delay(10);
           }
           packet_cnt = packet_cnt + 1.0;
        }
    }else if(swst  == 3.0){   //RELEASED Mode => start transmission

       //---GPS---
       // read data from the GPS in the 'main loop'
       c = GPS.read();

       // if a sentence is received, we can check the checksum, parse it...
       if (GPS.newNMEAreceived()) {
          if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
            return; // we can fail to parse a sentence in which case we should just wait for another
       }
                
       // if millis() or timer wraps around, we'll just reset it
       if (timer > millis()) timer = millis();

       // approximately every 1 second
       if (millis() - timer > 1000) { 
          timer = millis(); // reset the timer      

      
          bmp();   //temp, pressure and altitude readings
          voltageSensor();
          rgbLED();
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
          gps();
        
          //Updating values to save in EEPROM
        
          EEPROM.update(6,mission_t);
          EEPROM.update(10,packet_cnt);
          EEPROM.update(14,alt_i);
          EEPROM.update(18,pressure);
          EEPROM.update(22,temp);
          EEPROM.update(26,voltage);
          EEPROM.update(30,gps_th);
          EEPROM.update(34,gps_lat);
          EEPROM.update(38,gps_long);
          EEPROM.update(42,gps_alt);
          EEPROM.update(46,gps_sats);
          EEPROM.update(50,pitch);
          EEPROM.update(54,roll);
          EEPROM.update(58,rpm);
          EEPROM.update(62,swst);
          EEPROM.update(66, gps_tm);
          EEPROM.update(70, gps_ts);
          EEPROM.update(74, gyroXoffset); 
          EEPROM.update(78, gyroYoffset);
          EEPROM.update(82, gyroZoffset);

          //Convert data to sent through XBees
          imp();
          for(int i = 0; i < 92; ++i){
            Xbee.write(payload[i]);
            //delay(10);
          }
          packet_cnt = packet_cnt + 1.0;
          just_landed();      //checking if 
      }
    }else if(swst == 4.0){  //Seriously just landed
        voltageSensor();
        rgbLED();
        tone(buzzer, 1000);
        delay(4050);
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
  if(voltage > yellow){                             //GREEN   
        digitalWrite(greenPin, HIGH);
  }else if((voltage < yellow) && (voltage > red)){  //YELLOW
        digitalWrite(greenPin, HIGH);
        digitalWrite(redPin,HIGH);
  }else if(voltage < red){                          //RED
        digitalWrite(greenPin,LOW);
        delay(300);
        digitalWrite(redPin,HIGH);
  }
}
void rgbLED(){
  // EMPTY
}

void imuGyro(){
  float x;
  float y;
  float z;

  mpu6050.update();

  x = mpu6050.getAngleX();
  y = mpu6050.getAngleY();
  z = mpu6050.getAngleZ();
  
  pitch = 180 * atan(x/sqrt(y*y + z*z))/PI;
  roll = 180 * atan(y/sqrt(x*x + z*z))/PI;  
}

void imp(){
  
  FLOATUNION_t myfloat;

  variables[0] = 1000000; //0-3
  variables[1] = team_id; //5-8
  variables[2] = mission_t; //10-13
  variables[3] = packet_cnt; //15-18
  variables[4] = alt_i; //20-23
  variables[5] = pressure; //25-28
  variables[6] = temp; //30-33
  variables[7] = voltage; //35-38
  variables[8] = gps_th; //40-43
  variables[9] = gps_tm; //44-47
  variables[10] = gps_ts; //48-51
  variables[11] = gps_lat; //53-56
  variables[12] = gps_long; //58-61
  variables[13] = gps_alt; //63-66
  variables[14] = gps_sats; //68-71
  variables[15] = pitch; //73-76
  variables[16] = roll; //78-81
  variables[17] = rpm; //83-86
  variables[18] = swst; //88-91
  comma = ',';

  int i=0;
  int j=0;
  int cont = 0;
  for(i = 0; i < 19; i++){
    myfloat.number = variables[i];
    for(j = 0; j < 4; j++){
      payload[cont] = myfloat.bytes[j];
      ++cont;
    }
    if(i != 8 && i != 9 && i != 18){
      payload[cont] = comma;
      ++cont;
    }
  }
}

void gps(){

    gps_th = GPS.hour;
    gps_tm = GPS.minute;
    gps_ts = GPS.seconds;

    gps_lat = GPS.latitudeDegrees;
    gps_long = GPS.longitudeDegrees;
    gps_alt = GPS.altitude;  
    gps_sats = GPS.satellites;
}
