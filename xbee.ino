//Byte's vector
uint8_t payload[86];


//VARIABLES
//first, timeC, packet_count, altitude, pressure, temperature, voltage, gps_time_hour, gps_time_minute,
//gps_time_seconds, gps_latitude, gps_longitude, gps_altitude, gps_sats, pitch, roll, blade_spin_rate, 
//software_state

float variables[18];
uint8_t comma;


typedef union
{
  float number;
  uint8_t bytes[4];   
}FLOATUNION_t;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  
}

void loop() {
    
  variables[0] = 1000000; //first 0-3
  //comma 4
  variables[1] = 345 + random(10); //timeC 5-8
  //comma 9
  variables[2] = 43 + random(10); //packet_count 10-13
  //comma 14
  variables[3] = 63 + random(10); //altitude 15-18
  //comma 19
  variables[4] = 63 + random(10); //pressure 20-23
  //comma 24
  variables[5] = 23 + random(10); //temperature 25-28
  //comma 29
  variables[6] = 57 + random(10); //voltage 30-33
  //comma 34
  variables[7] = 53 + random(10); //gps_time_hour 35-38
  variables[8] = 43 + random(10); //gps_time_minute 39-42
  variables[9] = 76 + random(10); //gps_time_second 43-46
  //comma 47
  variables[10] = 53 + random(10); //gps_latitude 48-51
  //comma 52
  variables[11] = 34 + random(10); //gps_longitude 53-56
  //comma 57
  variables[12] = 43 + random(10); //gps_altitude 58-61
  //comma 62
  variables[13] = 34 + random(10); //gps_sats 63-66
  //comma 67
  variables[14] = 87 + random(10); //pitch 68-71
  //comma 72
  variables[15] = 74 + random(10); //roll 73-76
  //comma 77
  variables[16] = 43 + random(10); //blade_spin_rate 78-81
  //comma 82
  variables[17] = 56 + random(10); //software_state 83-86
  comma = ',';
  
  imp(); //float-byte function

  for(int i = 0; i < 86; ++i){
      Serial.write(payload[i]);
      //delay(10);
  }
  
  delay(500);
}



//Add data in byte format to the payload array
void imp(){
  
  FLOATUNION_t myfloat;
  
  int cont = 0;

  for(int i = 0; i < 18; ++i){
    myfloat.number = variables[i];
    for(int j = 0; j < 4; ++j){
      payload[cont] = myfloat.bytes[j];
      ++cont;
    }
    if(i != 7 && i != 8 && i != 17){
      payload[cont] = comma;
      ++cont;
    }
  }