#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define TriggerPin 4
#define PulsePin 7
#define LickPin 8

int Frequency = 0;
int PulseWidth = 5;
int Duration = 2000;
int PulseOff;
int8_t Condition = 1;

boolean LickCheck;

unsigned int trial_duration = 4500; // 4.5 seconds
unsigned int pre = 1500; // 1.5 seconds

unsigned long pulse_start_time;
unsigned long current_time;

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void setup() {
  pinMode(TriggerPin, INPUT);
  pinMode(PulsePin, OUTPUT);
  pinMode(LickPin, INPUT);
  digitalWrite(PulsePin, LOW);

  lcd.begin(16, 2);
  lcd.clear();

  boolean Adjustment = false;
  Adjustment = user_input(String("Make some adjust"), String("ment? No"), String("ment? Yes"), Adjustment);

  if (Adjustment) {
    boolean FrequencyAdjust = false;
    FrequencyAdjust = user_input(String("Change frequency"), String("? No"), String("? Yes"), FrequencyAdjust);
    uint8_t buttons = lcd.readButtons();
    if (FrequencyAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Frequency = ");
      lcd.print(Frequency);
      lcd.setCursor(0, 1);
      lcd.print("Change It!");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        while (buttons == lcd.readButtons()) {
        }
        if ((buttons & BUTTON_RIGHT) || (buttons & BUTTON_LEFT)) {
          if (buttons & BUTTON_RIGHT) {
            Frequency = Frequency + 5;
          }
          if (buttons & BUTTON_LEFT) {
            Frequency = Frequency - 5;
            if (Frequency < 0) {
              Frequency = 0;
            }
          }
          lcd.clear();
          lcd.print("Frequency = ");
          lcd.print(Frequency);
        }
      }
    }
    delay(1500);

    boolean PulseWidthAdjust = false;
    PulseWidthAdjust = user_input(String("Change pulse wid"), String("th? No"), String("th? Yes"), PulseWidthAdjust);
    if (PulseWidthAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("PulseWidth = ");
      lcd.print(PulseWidth);
      lcd.setCursor(0, 1);
      lcd.print("Change It!");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        while (buttons == lcd.readButtons()) {
        }
        if ((buttons & BUTTON_RIGHT) || (buttons & BUTTON_LEFT)) {
          if (buttons & BUTTON_RIGHT) {
            PulseWidth++;
            if (PulseWidth >= (int)(1.0 / Frequency * 1000.0)) {
              PulseWidth = (int)(1.0 / Frequency * 1000.0);
            }
          }
          if (buttons & BUTTON_LEFT) {
            PulseWidth--;
            if (PulseWidth <= 0) {
              PulseWidth = 1;
            }
          }
          lcd.clear();
          lcd.print("PulseWidth = ");
          lcd.print(PulseWidth);
        }
      }
    }
    delay(1500);

    boolean DurationAdjust = false;
    DurationAdjust = user_input(String("Change duration?"), String("No"), String("Yes"), DurationAdjust);
    if (DurationAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Duration = ");
      lcd.print(Duration);
      lcd.setCursor(0, 1);
      lcd.print("Change It!");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        if ((buttons & BUTTON_RIGHT) || (buttons & BUTTON_LEFT)) {
          if (buttons & BUTTON_RIGHT) {
            Duration = Duration + 100;
          }
          if (buttons & BUTTON_LEFT) {
            Duration = Duration - 100;
            if (Duration <= 0) {
              Duration = 100;
            }
          }
          lcd.clear();
          lcd.print("Duration = ");
          lcd.print(Duration);
        }
        delay(50);
      }
    }
    delay(1500);

    boolean ConditionAdjust = false;
    ConditionAdjust = user_input(String("Change condition"), String("? No"), String("? Yes"), ConditionAdjust);
    if (ConditionAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Condition = ");
      lcd.print(Condition);
      lcd.setCursor(0, 1);
      lcd.print("Change It!");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        if ((buttons & BUTTON_RIGHT) || (buttons & BUTTON_LEFT)) {
          if (buttons & BUTTON_RIGHT) {
            Condition = Condition + 1;
            if (Condition > 2) {
              Condition = 2;
            }
          }
          if (buttons & BUTTON_LEFT) {
            Condition = Condition - 1;
            if (Condition < 0) {
              Condition = 0;
            }
          }
          lcd.clear();
          lcd.print("Condition = ");
          lcd.print(Condition);
        }
        delay(50);
      }
    }
    delay(1500);
  }

  lcd.clear();
  lcd.print("F=");
  lcd.print(Frequency);
  lcd.print("; D=");
  lcd.print(Duration);
  lcd.setCursor(0, 1);
  lcd.print("PW=");
  lcd.print(PulseWidth);
  lcd.print("; CDT=");
  lcd.print(Condition);
  delay(1500);

  PulseOff = (int)(1.0 / Frequency * 1000.0) - PulseWidth;
}

void loop() {
  while (!digitalRead(TriggerPin)) {
  }
  switch (Condition) {
    case 0:
      pulse_start_time = millis();
      if (Frequency == 0) {
        digitalWrite(PulsePin, HIGH);
        while (millis() - pulse_start_time < Duration) {
        }
        digitalWrite(PulsePin, LOW);
      }
      else {
        while (millis() - pulse_start_time < Duration) {
          digitalWrite(PulsePin, HIGH);
          current_time = millis();
          while (millis() - current_time < PulseWidth) {
          }
          digitalWrite(PulsePin, LOW);
          current_time = millis();
          while (millis() - current_time < PulseOff) {
          }
        }
      }
      break;
    case 1:
      pulse_start_time = millis();
      if (Frequency == 0) {
        digitalWrite(PulsePin, HIGH);
        while (millis() - pulse_start_time < Duration && !digitalRead(LickPin)) {
        }
        digitalWrite(PulsePin, LOW);
      }
      else {
        LickCheck = false;
        while (millis() - pulse_start_time < Duration && !LickCheck && !digitalRead(LickPin)) {
          digitalWrite(PulsePin, HIGH);
          current_time = millis();
          while (millis() - current_time < PulseWidth) {
            if (digitalRead(LickPin)) {
              LickCheck = true;
              break;
            }
          }
          digitalWrite(PulsePin, LOW);
          current_time = millis();
          while (millis() - current_time < PulseOff && !LickCheck) {
            if (digitalRead(LickPin)) {
              LickCheck = true;
              break;
            }
          }
        }
      }
      break;
    case 2:
      current_time = millis();
      while (!digitalRead(LickPin) && millis() -  current_time < trial_duration - pre - Duration) {
      }
      while (digitalRead(LickPin) && millis() -  current_time < trial_duration - pre - Duration) {
      }
      if (millis() -  current_time < trial_duration - pre - Duration) {
        pulse_start_time = millis();
        if (Frequency == 0) {
          digitalWrite(PulsePin, HIGH);
          while (millis() - pulse_start_time < Duration && !digitalRead(LickPin)) {
          }
          digitalWrite(PulsePin, LOW);
        }
        else {
          LickCheck = false;
          while (millis() - pulse_start_time < Duration && !LickCheck && !digitalRead(LickPin)) {
            digitalWrite(PulsePin, HIGH);
            current_time = millis();
            while (millis() - current_time < PulseWidth) {
              if (digitalRead(LickPin)) {
                LickCheck = true;
                break;
              }
            }
            digitalWrite(PulsePin, LOW);
            current_time = millis();
            while (millis() - current_time < PulseOff && !LickCheck) {
              if (digitalRead(LickPin)) {
                LickCheck = true;
                break;
              }
            }
          }
        }
      }
      break;
    default:
      break;
  }
}

boolean user_input(String Sline1, String Sline2, String Sline3, boolean stats) {
  lcd.clear();
  lcd.print(Sline1);
  lcd.setCursor(0, 1);
  lcd.print(Sline2);
  int npress = 0;
  uint8_t buttons = lcd.readButtons();
  while ((buttons & BUTTON_SELECT)) {
    buttons = lcd.readButtons();
  }
  while (!(buttons & BUTTON_SELECT)) {
    buttons = lcd.readButtons();
    if (buttons) {
      while (buttons == lcd.readButtons()) {
      }
      if (!(buttons & BUTTON_SELECT)) {
        npress++;
        lcd.clear();
        lcd.print(Sline1);
        if (npress % 2) {
          lcd.setCursor(0, 1);
          lcd.print(Sline3);
          stats = !stats;
        }
        else {
          lcd.setCursor(0, 1);
          lcd.print(Sline2);
          stats = !stats;
        }
      }
    }
  }
  return stats;
}
