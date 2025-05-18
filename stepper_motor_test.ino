#define DIR 27
#define STEP 25
#define EN 26
#define CHOP 24

float rpm = 120;
uint8_t microsteps_z = 64;

void setup() {
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(CHOP, OUTPUT);
  digitalWrite(EN, LOW);
  digitalWrite(STEP, LOW);
  digitalWrite(CHOP, HIGH);
  float period = 1 / (rpm / 60 * 200 * microsteps_z) * pow(10, 6);
  for (int j = 0; j < 2; j++) {
    delay(2000);
    digitalWrite(DIR, HIGH);
    for (int i = 0; i < 200*microsteps_z * 3; i++) {
      digitalWrite(STEP, HIGH);
      delayMicroseconds(4);
      digitalWrite(STEP, LOW);
      delayMicroseconds(period-4);
    }
    delay(2000);
    digitalWrite(DIR, LOW);
    for (int i = 0; i < 200*microsteps_z * 3; i++) {
      digitalWrite(STEP, HIGH);
      delayMicroseconds(4);
      digitalWrite(STEP, LOW);
      delayMicroseconds(period-4);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
