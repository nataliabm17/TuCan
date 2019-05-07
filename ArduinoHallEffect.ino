 volatile byte half_revolutions;
 unsigned int rpm;
 unsigned long timeold;
 void setup()
 {
   Serial.begin(9600);
   attachInterrupt(digitalPinToInterrupt(21), magnet_detect, RISING);//Initialize the intterrupt pin (Arduino digital pin 2)
   half_revolutions = 0;
   rpm = 0;
   timeold = 0;
 }
 void loop() //Measure RPM
 {
   if (half_revolutions >= 3) { 
     rpm = (60000*half_revolutions)/(millis() - timeold);
     timeold = millis();
     half_revolutions = 0;
     Serial.println(rpm);
   }
 }
 void magnet_detect()//This function is called whenever a magnet/interrupt is detected by the arduino
 {
   half_revolutions++;
 //  Serial.println("detect");
 }
