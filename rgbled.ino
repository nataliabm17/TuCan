float yellow = 7.2;
float red = 6.0;

int redPin = 3;
int bluePin = 4;
int greenPin = 5;

void setup(){
    pinMode(redPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(greenPin, OUTPUT);
}

void rgbLED(){
    if(voltage > yellow){                             //GREEN   
        digitalWrite(greenPin, HIGH);
    }else if((voltage < yellow) && (voltage > red)){  //YELLOW
        digitalWrite(greenPin, HIGH);
        digitlWrite(redPin,HIGH);
    }else if(voltage < red){                          //RED
        digitalWrite(greenPin,LOW);
        delay(300);
        digitalWrite(redPin,HIGH);
    }
}
