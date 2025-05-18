#include <Wire.h>
#define Lickometer A0

float lickstate;

void setup() {
  Serial.begin(9600);
}

void loop() {
  lickstate = analogRead(Lickometer) / 1024.0 * 5.0;
  Serial.println(lickstate);
}
