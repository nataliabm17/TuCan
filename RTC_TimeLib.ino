#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

tmElements_t tm;

float mission_t = 0;

void settingTime();
void readingTime();

void setup() {
  bool parse=false;
  bool config=false;

  // get the time the compiler was run
  //if (getTime(__TIME__)) {
    //parse = true;
    // and configure the RTC with this info
    //if (RTC.write(tm)) {
      //config = true;
    //}
  //}
  settingTime();
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  delay(200);
}

void loop() {
  //RTC.read(tm);
  //Serial.print(tm.Hour);
  //Serial.print(", ");
  //Serial.print(tm.Minute);
  //Serial.print(", ");
  //Serial.print(tm.Second);
  //Serial.print("\n");
  readingTime();
}

bool getTime(const char *str){
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}


void settingTime(){
  tm.Hour = 1;
  tm.Minute = 0;
  tm.Second = 0;
}

void readingTime(){
  RTC.read(tm);
  mission_t = (60*tm.Minute)+tm.Second;
  
}
