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
#define LickPin 27

unsigned long session_duration = 1800000; // 30 minutes
unsigned int trial_duration = 4500; // 4.5 seconds
unsigned int pre = 1500; // 1.5 seconds
unsigned int current_pre;

unsigned int stimulation_delay = 0; // delay from valve onset
uint8_t stim_every_n_trials = 3; // one stimulation in n trials
uint8_t StimulationTrial;
uint8_t trialCount = 0;

unsigned int time_valve_open = 32; // 33 ms, 1.61; 32 ms, 1.38
unsigned int time_buzzer_on = 100; // 0.1 second
unsigned int TTL_width_for_PulseGenerator = 5; // 5 ms TTL pulse for PulseGenerator

boolean TimeOut = false;
boolean Conditioning = false;
boolean isWaterValveOpen;
boolean isStimOn;

enum State {
  INIT,
  LED_INDICATOR_ON_OFF,
  PRE_TRIAL,
  VALVE_ON,
  STIMULATION,
  WAIT_TRIAL_END,
  TIMEOUT
};

State currentState;
unsigned long SessionStartTime;
unsigned long TrialStartTime;
unsigned long WaterValveStartTime;
unsigned long StimulationStartTime;

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

  SessionStartTime = millis();
  currentState = INIT;
}

void loop() {
  switch (currentState) {
    case INIT:
      CreateTrulyRandomSeed();
      myRandomSeed(seed);
      StimulationTrial = random(stim_every_n_trials);
      trialCount = 0;
      currentState = LED_INDICATOR_ON_OFF;
      break;

    case LED_INDICATOR_ON_OFF:
      digitalWrite(LED_Indicator, (StimulationTrial == trialCount) ? HIGH : LOW);
      TrialStartTime = millis();
      currentState = PRE_TRIAL;
      break;

    case PRE_TRIAL:
      if (digitalRead(LickPin)){
        TrialStartTime = millis();
      }
      if (millis() - TrialStartTime >= pre) {
        digitalWrite(WaterValve, HIGH);
        if (StimulationTrial == trialCount && Conditioning) {
          tone(PiezoBuzzer, 12000, time_buzzer_on);
        }
        WaterValveStartTime = millis();
        isWaterValveOpen = true;
        currentState = VALVE_ON;
      }
      break;

    case VALVE_ON:
      if (StimulationTrial != trialCount) {
        if (millis() - WaterValveStartTime >= time_valve_open) {
          digitalWrite(WaterValve, LOW);
          isWaterValveOpen = false;
          currentState = WAIT_TRIAL_END;
        }
      } else {
        if (time_valve_open < stimulation_delay) {
          if (millis() - WaterValveStartTime >= time_valve_open && isWaterValveOpen) {
            digitalWrite(WaterValve, LOW);
            isWaterValveOpen = false;
          }
        }
        if (millis() - WaterValveStartTime >= stimulation_delay) {
          digitalWrite(PulseGenerator, HIGH);
          StimulationStartTime = millis();
          isStimOn = true;
          currentState = STIMULATION;
        }
      }
      break;

    case STIMULATION:
      if (time_valve_open >= stimulation_delay) {
        if (millis() - WaterValveStartTime >= time_valve_open && isWaterValveOpen) {
          digitalWrite(WaterValve, LOW);
          isWaterValveOpen = false;
        }
      }
      if (millis() - StimulationStartTime >= TTL_width_for_PulseGenerator && isStimOn) {
        digitalWrite(PulseGenerator, LOW);
        isStimOn = false;
      }
      if (!isWaterValveOpen && !isStimOn) {
        currentState = WAIT_TRIAL_END;
      }
      break;

    case WAIT_TRIAL_END:
      if (millis() - TrialStartTime >= trial_duration) {
        if (millis() - SessionStartTime >= session_duration) {
          currentState = TIMEOUT;
        } else {
          trialCount++;
          if (trialCount >= stim_every_n_trials) {
            currentState = INIT;
          } else {
            currentState = LED_INDICATOR_ON_OFF;
          }
        }
      }
      break;

    case TIMEOUT:
      if (!TimeOut) {
        digitalWrite(IRLED, LOW);
        digitalWrite(LED_Indicator, LOW);
        TimeOut = true;
      }
      break;
  }
}

void CreateTrulyRandomSeed() {
  seed = 0;
  nrot = 32;

  cli();
  MCUSR = 0;
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDIE);
  sei();

  while (nrot > 0);

  cli();
  MCUSR = 0;
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (0 << WDE);
  _WD_CONTROL_REG = (0 << WDIE);
  sei();
}

ISR(WDT_vect) {
  nrot--;
  seed = (seed << 8) ^ TCNT1L;
}
