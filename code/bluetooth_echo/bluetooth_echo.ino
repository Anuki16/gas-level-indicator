/* Turn an LED on/off based on a command send via BlueTooth
**
** Credit: The following example was used as a reference
** Rui Santos: http://randomnerdtutorials.wordpress.com
*/
#include <SoftwareSerial.h>

int ledPin = 13;  // use the built in LED on pin 13 of the Uno
int rxPin = 4;
int txPin = 2;
int state = 0;
int flag = 0;        // make sure that you return the state only once

SoftwareSerial btSerial(rxPin, txPin);

void setup() {
    // sets the pins as outputs:
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    Serial.begin(9600); // Default connection rate for my BT module
    btSerial.begin(9600);
}

void loop() {
    //if some data is sent, read it and save it in the state variable
    if(btSerial.available() > 0){
      state = btSerial.read();
      flag=0;
    }
    // if the state is 0 the led will turn off
    if (state == '0') {
        digitalWrite(ledPin, LOW);
        if(flag == 0){
          Serial.println("LED: off");
          btSerial.println("LED: off");
          flag = 1;
        }
    }
    // if the state is 1 the led will turn on
    else if (state == '1') {
        digitalWrite(ledPin, HIGH);
        if(flag == 0){
          Serial.println("LED: on");
          btSerial.println("LED: on");
          flag = 1;
        }
    }
}
