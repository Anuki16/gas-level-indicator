#include <HX711_ADC.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "storage.h"

//pins:
#define ZERO_BUTTON 12
#define RED_LED 9
#define YELLOW_LED 10
#define GREEN_LED 11

#define HX711_DOUT 4
#define HX711_SCK 5
#define BT_RX 2   //Connect this pin to Bluetooth TXD
#define BT_TX 3   //To RXD

SoftwareSerial BTserial(BT_RX, BT_TX);  //RX, TX
HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);

void data_read_all() {  //read all data and empty array
  float data;
  while (!data_is_empty()) {
    data = data_get();
    BTserial.print(data);
    BTserial.print(",");
  }
  BTserial.println();
}

float cal_factor = 23.8;
long tare_offset = 8569861;

const int cal_eeprom = 0;
unsigned long t = 0;  //timestamp
float empty_weight = 2500.0;
float full_gas_weight = 7500.0;
//total weight = 20

int cur_led = 0;

void calibrate() {
  Serial.println("Remove weights to tare and press t");
  while (1) {
    if (Serial.available() > 0) {
      char command = Serial.read();
      if (command == 't') {
        LoadCell.tare();
        Serial.print("Tare complete. Tare offset: ");
        long offset = LoadCell.getTareOffset();
        Serial.println(offset);
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

  Serial.println("Save this and press k");
  while (1) {
    if (Serial.available() > 0) {
      char command = Serial.read();
      if (command == 'k') {
        break;
      }
    }
  }
}

void calibrate_bluetooth() {
  BTserial.println("Remove weights to tare and press t");
  while (1) {
    if (BTserial.available() > 0) {
      char command = BTserial.read();
      if (command == 't') {
        LoadCell.tare();
        BTserial.print("Tare complete. Tare offset: ");
        long offset = LoadCell.getTareOffset();
        BTserial.println(offset);
        break;
      }
    }
  }

  //place known mass
  float known_mass = 0;
  BTserial.println("Place known mass and enter its weight in grams");
  while (1) {
    if (BTserial.available() > 0) {
      known_mass = BTserial.parseFloat();
      if (known_mass != 0) {
        BTserial.print("Known mass is: ");
        BTserial.println(known_mass);
        break;
      }
    }
  }
  LoadCell.refreshDataSet();
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass);
  BTserial.print("New calibration value has been set to: ");
  BTserial.println(newCalibrationValue);
  cal_factor = newCalibrationValue;

  BTserial.println("Save this to EEPROM address 0? y/n");
  while (1) {
    if (BTserial.available() > 0) {
      char command = BTserial.read();
      if (command == 'y') {
        EEPROM.put(cal_eeprom, newCalibrationValue);
        BTserial.println("Saved");
      } 
      break;  //break if anything is sent
    }
  }
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
  if (perc < 35) next_led = RED_LED;
  if (perc >= 35 && perc < 65) next_led = YELLOW_LED;
  if (perc >= 65) next_led = GREEN_LED;

  if (next_led != cur_led) {
    change_led(cur_led, next_led);
    cur_led = next_led;
  }
}

void light_zero_led() {
  light_led(0);
  delay(200);
  light_led(50);
  delay(200);
  light_led(100);
  delay(200);
  light_led(0);
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
  BTserial.begin(9600);
  LoadCell.begin();
  //calibrate();  // comment this and uncomment below lines after scale is calibrated

  float saved_calfactor;
  EEPROM.get(cal_eeprom, saved_calfactor);

  if (22 <= saved_calfactor && saved_calfactor <= 26) {
    cal_factor = saved_calfactor;
  } 
  LoadCell.setCalFactor(cal_factor);
  LoadCell.setTareOffset(tare_offset);
}

void loop() {
  // put your main code here, to run repeatedly:
  static boolean newDataReady = 0;
  const int serialPrintInterval = 1000; //increase value to slow down serial print activity

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
  if (digitalRead(ZERO_BUTTON) == HIGH) {
    light_zero_led();
    LoadCell.tare();
    Serial.print("Tare complete. Tare offset: ");
    long offset = LoadCell.getTareOffset();
    Serial.println(offset);
  }

  if (BTserial.available())
  {
    char comm = BTserial.read();
    if (comm == 'd') {
      data_read_all();
      Serial.println("Data sent");

    } else if (comm == 'i') {
      full_gas_weight = BTserial.parseFloat();
      empty_weight = BTserial.parseFloat();
      Serial.println("Input parameters");
      Serial.print("Gas weight: ");
      Serial.println(full_gas_weight);
      Serial.print("Empty weight: ");
      Serial.println(empty_weight);
    }
//    } else if (comm = 'f') {
//      BTserial.print("Calibration factor: ");
//      BTserial.println(cal_factor);
//      
//    } 
      else if (comm = 'c') {
      calibrate_bluetooth();
    }
  }
}
