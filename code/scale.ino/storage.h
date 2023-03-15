#ifndef STORAGE_H
#define STORAGE_H

#define QUEUE_LEN 128

float data_queue[QUEUE_LEN];
byte rp = 0;
byte wp = 0;
unsigned int count = 0;

void data_put(float data) {
  data_queue[wp] = data;  // will overwrite oldest data
  wp++;
  count++;
  if (wp >= QUEUE_LEN) wp = 0;  
  if (count > QUEUE_LEN) {  // old data has been overwritten, move read pointer and update count
    count = QUEUE_LEN;
    rp++;
    if (rp >= QUEUE_LEN) rp = 0;
  }
}

float data_get() {
  if (count == 0) { //queue empty
    return 0;
  }
  float val = data_queue[rp];
  rp++;
  count--;
  if (rp >= QUEUE_LEN) rp = 0;
  return val;
}

int data_is_empty() { //check this before getting data
  return count == 0;
}

void data_read_all() {  //read all data and empty array
  float data;
  while (!data_is_empty()) {
    data = data_get();
    Serial.print(data);
    Serial.print(" ");
  }
  Serial.println();
}

#endif
