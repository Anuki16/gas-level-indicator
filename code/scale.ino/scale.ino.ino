#include <HX711_ADC.h>
#include <EEPROM.h>
#include "storage.h"

//pins:
#define ZERO_BUTTON 8
#define RED_LED 11
#define YELLOW_LED 12
#define GREEN_LED 13

const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
float cal_factor = 36059.35;
long tare_offset = 8460732;

unsigned long t = 0;  //timestamp
float empty_weight = 5.0;
float full_gas_weight = 15.0;
//total weight = 20

int cur_led = 0;

void calibrate() {
  Serial.println("Remove weights to tare and press t");
  while (1) {
    if (Serial.available() > 0) {
      char command = Serial.read();
      if (command == 't') {
        LoadCell.tare();
        break;
      }
    }
  }
  
  //place known mass
  float known_mass = 0;
  Serial.println("Place known mass and enter its weight in grams: ");
  while (1) {
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        break;
      }
    }
  }
  LoadCell.refreshDataSet();
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass);
  Serial.print("New calibration value has been set to: ");
  Serial.println(newCalibrationValue);
  cal_factor = newCalibrationValue;
}

float calc_percentage(float weight) {
  float gas_weight = weight - empty_weight;
  float perc = gas_weight * 100 / full_gas_weight;
  return perc;
}

void change_led(int cur, int next) {
  if (cur != 0) digitalWrite(cur, LOW);
  if (next != 0) digitalWrite(next, HIGH);
}

void light_led(float perc) {
  int next_led = cur_led;
  if (perc < 0) next_led = 0;
  if (perc >= 0 && perc < 35) next_led = RED_LED;
  if (perc >= 35 && perc < 65) next_led = YELLOW_LED;
  if (perc >= 65) next_led = GREEN_LED;

  if (next_led != cur_led) {
    change_led(cur_led, next_led);
    cur_led = next_led;
  }
}

void process_new_data(float weight) {
  data_put(weight);
  float perc = calc_percentage(weight);
  light_led(perc);
  Serial.print("Remaining gas percentage: ");
  Serial.print(perc);
  Serial.println("%");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(ZERO_BUTTON, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  Serial.begin(9600);
  LoadCell.begin();
  //calibrate();
  LoadCell.setCalFactor(cal_factor);
  LoadCell.setTareOffset(tare_offset);
}

void loop() {
  // put your main code here, to run repeatedly:
  static boolean newDataReady = 0;
  const int serialPrintInterval = 2000; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(weight);
      process_new_data(weight);
      newDataReady = false;
      t = millis();
    }
  }

  // If tare button is pressed, do tare operation
  if (digitalRead(ZERO_BUTTON) == LOW) {
    LoadCell.tare();
    Serial.print("Tare complete. Tare offset: ");
    long offset = LoadCell.getTareOffset();
    Serial.println(offset);
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 'r') data_read_all();
  }
  
}
