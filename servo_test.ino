#include <Servo.h>
#define ServoPin 9
#define ServoPulse_Min 600
#define ServoPulse_Max 2400
Servo Actuator;

void setup() {
  // put your setup code here, to run once:
  Actuator.attach(ServoPin, ServoPulse_Min, ServoPulse_Max);
}

void loop() {
  // put your main code here, to run repeatedly:
  Actuator.writeMicroseconds(1500);
  delay(1000);
  Actuator.writeMicroseconds(1615);
  delay(1000);
}
