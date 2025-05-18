#define WaterValve 5
#define LED_Indicator 24

unsigned int time_valve_open = 32;
unsigned int times = 500;
unsigned int counter = 0;

void setup() {
  pinMode(WaterValve, OUTPUT);
  digitalWrite(WaterValve, LOW);
  pinMode(LED_Indicator, OUTPUT);
  digitalWrite(LED_Indicator, LOW);
  delay(10000);
}

void loop() {
  if (counter < 500) {
    digitalWrite(WaterValve, HIGH);
    unsigned long valve_start_time = millis();
    while (millis() - valve_start_time < time_valve_open) {
    }
    digitalWrite(WaterValve, LOW);
    delay(50);
    counter++;
  }
  else {
    digitalWrite(LED_Indicator, HIGH);
  }
}
