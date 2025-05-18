#define DIR_Z 26
#define STEP_Z 27

float rpm_z = 300;
float rpm_z_accstart = 300;
float accduration_z = 0.4;
uint8_t microsteps_z = 16;

int steps_pellet = 600;

void setup() {
  Serial.begin(9600);
  pinMode(DIR_Z, OUTPUT);
  pinMode(STEP_Z, OUTPUT);
  digitalWrite(STEP_Z, LOW);
  stepz((unsigned long) steps_pellet * (unsigned long) microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 0);
  delay(2000);
  stepz((unsigned long) steps_pellet * (unsigned long) microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 1);
}

void loop() {

}

void stepz(unsigned long zsteps, float acceleration_duration, float zspeed_initial, float zspeed, boolean zdirection) {
  float acceleration;
  int pulse_interval;
  float speed_tmp = zspeed_initial;
  int count = 0;
  if (zdirection) {
    digitalWrite(DIR_Z, LOW);
  }
  else {
    digitalWrite(DIR_Z, HIGH);
  }
  acceleration = (zspeed - zspeed_initial) / acceleration_duration;
  while (speed_tmp < zspeed) {
    speed_tmp = acceleration * 1 / (speed_tmp / 60.0 * 200 * microsteps_z) + speed_tmp;
    count++;
  }
  if (zsteps <= count + 1) {
    for (int i = 0; i < zsteps; i++) {
      digitalWrite(STEP_Z, HIGH);
      delayMicroseconds(4);
      digitalWrite(STEP_Z, LOW);
      pulse_interval = int (pow(10, 6) / (zspeed_initial / 60.0 * 200 * microsteps_z) - 4);
      delayMicroseconds(pulse_interval);
      zspeed_initial = acceleration * 1 / (zspeed_initial / 60.0 * 200 * microsteps_z) + zspeed_initial;
    }
  }
  else if (zsteps <= 2 * count + 1) {
    for (int i = 0; i < zsteps; i++) {
      digitalWrite(STEP_Z, HIGH);
      delayMicroseconds(4);
      digitalWrite(STEP_Z, LOW);
      pulse_interval = int (pow(10, 6) / (zspeed_initial / 60.0 * 200 * microsteps_z) - 4);
      delayMicroseconds(pulse_interval);
      if (i < count) {
        zspeed_initial = acceleration * 1 / (zspeed_initial / 60.0 * 200 * microsteps_z) + zspeed_initial;
      }
      else {
        zspeed_initial = -acceleration * 1 / (zspeed_initial / 60.0 * 200 * microsteps_z) + zspeed_initial;
      }
    }
  }
  else {
    for (int i = 0; i < zsteps; i++) {
      digitalWrite(STEP_Z, HIGH);
      delayMicroseconds(4);
      digitalWrite(STEP_Z, LOW);
      pulse_interval = int (pow(10, 6) / (zspeed_initial / 60.0 * 200 * microsteps_z) - 4);
      delayMicroseconds(pulse_interval);
      if (i < count) {
        zspeed_initial = acceleration * 1 / (zspeed_initial / 60.0 * 200 * microsteps_z) + zspeed_initial;
      }
      else if (i >= zsteps-count-1) {
        zspeed_initial = -acceleration * 1 / (zspeed_initial / 60.0 * 200 * microsteps_z) + zspeed_initial;
      }
      
    }
  }
}
