//Written by Ahmet Burkay KIRNIK
//TR_CapaFenLisesi
//Measure Angle with a MPU-6050(GY-521)

#include<Wire.h>
#include<MPU6050.h>
MPU6050 mpu;

const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

int minVal=265;
int maxVal=402;

double x;
double y;
double z;
 


void setup(){

  Wire.begin();
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
}
void loop(){
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
     
     Serial.print("Roll= ");
     Serial.println(x);

     Serial.print("Pitch= ");
     Serial.println(y);
     Serial.println("-----------------------------------------");
     delay(400);
}
