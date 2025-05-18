#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#define myRandomSeed(s) srandom(s)

volatile uint32_t seed;
volatile int8_t nrot;

#define WaterValve 5
#define PiezoBuzzer 4
#define PulseGenerator 25
#define IRLED 26
#define LED_Indicator 24

unsigned long duration = 1800000; // 30 minutes
unsigned long time_begin;
unsigned int trial_duration = 4500; // 4.5 seconds
unsigned int pre = 1500; // 1.5 seconds

unsigned int laser_delay = 200; // delay from valve onset
uint8_t n = 3; // one stimulation in n trials

unsigned int time_valve_open = 33; // 20 ms, 0.8 ul; 30 ms, 1.38 ul; 25 ms, 1.06 ul; 34 ms, 1.63; 33 ms, 1.52
unsigned int time_buzzer_on = 100; // 0.1 second

boolean TimeOut = false;
boolean Conditioning = true;

void setup() {
  pinMode(WaterValve, OUTPUT);
  digitalWrite(WaterValve, LOW);
  pinMode(PulseGenerator, OUTPUT);
  digitalWrite(PulseGenerator, LOW);
  pinMode(IRLED, OUTPUT);
  digitalWrite(IRLED, HIGH);
  pinMode(LED_Indicator, OUTPUT);
  digitalWrite(LED_Indicator, LOW);
  pinMode(PiezoBuzzer, OUTPUT);

  time_begin = millis();
}

void loop() {
  while (millis() - time_begin < duration) {
    CreateTrulyRandomSeed();
    myRandomSeed(seed);
    uint8_t LightTrial = random(n);
    for (uint8_t i = 0; i < n; i++) {
      if (LightTrial == i) {
        digitalWrite(LED_Indicator, HIGH);
      }
      else {
        digitalWrite(LED_Indicator, LOW);
      }
      CreateTrulyRandomSeed();
      myRandomSeed(seed);
      unsigned int current_pre = random(pre);
      unsigned long trial_start_time = millis();
      while (millis() - trial_start_time < current_pre) {
      }
      digitalWrite(WaterValve, HIGH);
      unsigned long valve_start_time = millis();
      if (LightTrial == i) {
        if (Conditioning) {
          tone(PiezoBuzzer, 12000, time_buzzer_on); // 12 kHz sound or 3 KHz
        }
        if (time_valve_open < laser_delay) {
          while (millis() - valve_start_time < time_valve_open) {
          }
          digitalWrite(WaterValve, LOW);
          while (millis() - valve_start_time < laser_delay) {
          }
          digitalWrite(PulseGenerator, HIGH);
          unsigned long pulse_start_time = millis();
          while (millis() - pulse_start_time < 5) {
          }
          digitalWrite(PulseGenerator, LOW);
        }
        else {
          while (millis() - valve_start_time < laser_delay) {
          }
          digitalWrite(PulseGenerator, HIGH);
          unsigned long pulse_start_time = millis();
          while (millis() - pulse_start_time < 5) {
          }
          digitalWrite(PulseGenerator, LOW);
          while (millis() - valve_start_time < time_valve_open) {
          }
          digitalWrite(WaterValve, LOW);
        }
      }
      else {
        while (millis() - valve_start_time < time_valve_open) {
        }
        digitalWrite(WaterValve, LOW);
      }
      while (millis() - trial_start_time < trial_duration) {
      }
    }
  }

  if (!TimeOut) {
    digitalWrite(IRLED, LOW);
    digitalWrite(LED_Indicator, LOW);
    TimeOut = true;
  }
}

void CreateTrulyRandomSeed()
{
  seed = 0;
  nrot = 32; // Must be at least 4, but more increased the uniformity of the produced
  // seeds entropy.

  // The following five lines of code turn on the watch dog timer interrupt to create
  // the seed value
  cli();
  MCUSR = 0;
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDIE);
  sei();

  while (nrot > 0);  // wait here until seed is created

  // The following five lines turn off the watch dog timer interrupt
  cli();
  MCUSR = 0;
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (0 << WDE);
  _WD_CONTROL_REG = (0 << WDIE);
  sei();
}

ISR(WDT_vect)
{
  nrot--;
  seed = seed << 8;
  seed = seed ^ TCNT1L;
}
