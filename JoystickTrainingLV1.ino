// JoyStickTrainingLV1
// threA = Threshould for Event Zone
// threB = Threshould for Reward Zone
// threC = Threshould for Overshoot Zone
// TargtP = Target Position
// RangAccept = Range
// JoyState: 0, baseline; 1, Event Zone; 2, Reward Zone; 3, Overshoot Zone
#define JoyPoPin A1 // Read Joystick Position from Analog Pin 1
#define RewardPin 1 // Store in NI Card
#define FailurePin 2 // Store in NI Card
#define NISynPin 3 // Use to enable NI, Log Behavior Camera, Log Imaging Camera Store
#define PiezoBuzzerPin 8

float threA = 1;// for example 0.5 mm in real world
float TargetP = 2;// Target, for exapmle 5 mm in real world
float RangeAccept = 0.5;// Range, for exapmle 1 mm in real world
float voltageconst = 5.0/pow(2, 10); //10 bit resolution for Mega
int threB =  TargetP - RangeAccept;
int threC =  TargetP + RangeAccept;

int NumTrials = 100 ;// Define how many trials you will go
int JoyState = 0;// Set it to baseline
int JoyPo = 0; // Variable to hold analog reading

int HoldDura = 500;
unsigned long TrainTimeStart; //
unsigned long TrialStartTime;
unsigned long TimeTotal = 1800000; //30 min
unsigned long MaxTrialDura = 300000; // 5 min
unsigned long DetectTrialStartCount;

boolean DetectTrialStart;
boolean SessionOver = false;

void setup() {
  pinMode (JoyPoPin, INPUT);
  pinMode (RewardPin, OUTPUT);
  pinMode (FailurePin, OUTPUT);
  pinMode (NISynPin, OUTPUT);
  TrainTimeStart = millis();
}
void loop() {
  // 1. Detect the condition for trial start
  for (int i = 1; i < NumTrials + 1 && (millis() - TrainTimeStart <= TimeTotal); i++) {
    DetectTrialStart = false;
    while (!DetectTrialStart) {
      DetectTrialStartCount = millis();
      while (analogRead (JoyPoPin)*voltageconst <= threA) {
        if (millis() - DetectTrialStartCount >= HoldDura) {
          break;
        }
      }
      if (millis() - DetectTrialStartCount < HoldDura) {
        continue;
      }

      //2. Trial start
      digitalWrite (NISynPin, HIGH); //enable NI, Behavior Camera, Imaging Camera

      TrialStartTime = millis();
      while (analogRead (JoyPoPin)*voltageconst <= threA && millis() - TrialStartTime < MaxTrialDura) {
        while (analogRead (JoyPoPin)*voltageconst > threA && millis() - TrialStartTime < MaxTrialDura) {
          JoyState = 1;
          while (analogRead (JoyPoPin)*voltageconst > threB && millis() - TrialStartTime < MaxTrialDura) {
            JoyState = 2;
            while (analogRead (JoyPoPin)*voltageconst > threC && millis() - TrialStartTime < MaxTrialDura) {
              JoyState = 3;
            }
          }
        }
      }
      if (JoyState == 2) {
        delay (1000);
        digitalWrite (RewardPin, HIGH);
        delay(10);
        digitalWrite (RewardPin, LOW);
        delay (2990);
        digitalWrite (NISynPin, LOW); //
      }
      else if (JoyState == 1 && JoyState == 3 && millis() - TrialStartTime >= MaxTrialDura) {
        delay (1000);
        digitalWrite(FailurePin, HIGH);
        delay(10);
        digitalWrite (FailurePin, LOW);
        delay (2990);
        digitalWrite (NISynPin, LOW); //
      }
      DetectTrialStart = true;// why??
      JoyState = 0;
    }
  }

  if (!SessionOver) {
    tone(PiezoBuzzerPin, 310, 5000);
  }
  SessionOver = true;
}
