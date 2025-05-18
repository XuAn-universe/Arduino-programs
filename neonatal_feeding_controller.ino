#define CameraTriggerPin 5
#define PelletDispenserPin 4

unsigned long time_interval = 5400000;  // 5400000, 1.5 hours
unsigned long time2record = 900000;             // 900000, 15 minutes
unsigned long time2increase_pellet = 86400000;  // 86400000, 24 hours
unsigned long time2compare_inner;
unsigned long time2compare_outer;

uint8_t npellet = 50; // 50
unsigned int pellet_interval = 1000;

void setup() {
  pinMode(CameraTriggerPin, OUTPUT);
  digitalWrite(CameraTriggerPin, LOW);
  pinMode(PelletDispenserPin, OUTPUT);
  digitalWrite(PelletDispenserPin, LOW);
}

void loop() {
  time2compare_outer = millis();
  while (millis() - time2compare_outer < time2increase_pellet) {
    time2compare_inner = millis();
    digitalWrite(CameraTriggerPin, HIGH);
    for (uint8_t i = 0; i < npellet; i++) {
      digitalWrite(PelletDispenserPin, HIGH);
      delay(pellet_interval / 2);
      digitalWrite(PelletDispenserPin, LOW);
      delay(pellet_interval / 2);
    }
    digitalWrite(CameraTriggerPin, LOW);
    while (millis() - time2compare_inner <= time2record) {
    }
    digitalWrite(CameraTriggerPin, HIGH);
    delay(1000);
    digitalWrite(CameraTriggerPin, LOW);
    while (millis() - time2compare_inner <= time_interval) {
    }
  }
  npellet = npellet + 10;
}