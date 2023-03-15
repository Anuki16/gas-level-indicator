#include <SoftwareSerial.h>
#include "storage.h"

SoftwareSerial BTserial(2, 3);  //RX, TX
unsigned long t = 0;
float val = 25.0;

void data_read_all() {  //read all data and empty array
  float data;
  while (!data_is_empty()) {
    data = data_get();
    BTserial.print(data);
    BTserial.print(",");
  }
  BTserial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // HC-06 default serial speed is 9600
  BTserial.begin(9600);
}

void loop() {
  if (BTserial.available())
  {
    char comm = BTserial.read();
    if (comm == 'd') {
      data_read_all();
      Serial.println("Data sent");
    } else if (comm == 'i') {
      float gas_weight = BTserial.parseFloat();
      float empty_weight = BTserial.parseFloat();
      Serial.println("Input parameters");
      Serial.print("Gas weight: ");
      Serial.println(gas_weight);
      Serial.print("Empty weight: ");
      Serial.println(empty_weight);
    }
  }
  if (millis() > t + 5000) {
    val -= random(20)/10.0;
    if (val < 0) val = random(15, 30);
    data_put(val);
    Serial.println(val);
    t = millis();
  }

}
