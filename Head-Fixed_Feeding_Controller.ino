#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Entropy.h>

#define LimitSwitch_Zsandpaper 6
#define DIR_Zsandpaper 5
#define STEP_Zsandpaper 4 //PG5

#define LeverL 22
#define LeverR 23

#define PiezoBuzzer 8

#define WaterValve 24
#define Lickometer A0

#define LaserTrigger 25

#define ACpowerTrigger 32

#define TrialStartTrigger 35

#define LimitSwitch_Y 28
#define LimitSwitch_Z 29
#define DIR_Y 26
#define STEP_Y 27 //PA5
#define DIR_Z 30
#define STEP_Z 31 //PC6

#define DIR_ObjectPlate 33
#define STEP_ObjectPlate 34 //PC3

float rpm_y = 90;
float rpm_y_homing = 1;
float rpm_y_accstart = 5;
float accduration_y = 0.5;
float rpm_y_adjust = 15;
uint8_t microsteps_y = 32;
float rpm_z = 60;
float rpm_z_homing = 1;
float rpm_z_accstart = 10;
float accduration_z = 1;
float rpm_z_adjust = 15;
uint8_t microsteps_z = 32;
int steps_y_load = 20;
int steps_z_load = 115;
int steps_y_docking = 362;
int steps_y_food2mouse = 587;
int steps_z_food2mouse = 304;
int steps_y_lever2mouse = 551;
int steps_z_lever2mouse = 188;
int steps_y_lickometer2mouse = 556;
int steps_z_lickometer2mouse = 302;
int steps_z_food = 60;

float rpm_objectplate = 6;
float rpm_objectplate_accstart = 6;
float accduration_objectplate = 0.5;
float rpm_objectplate_adjust = 3;
uint8_t microsteps_objectplate = 16;

int time_sandpaper_stay = 1000;
int time_food_stay = 5000;
int time_water_stay = 10000;
int time_eating = 5000;
int time_valve_open = 40;

float rpm_zsandpaper = 300;
float rpm_zsandpaper_homing = 0.1;
float rpm_zsandpaper_accstart = 300;
float accduration_zsandpaper = 0.5;
float rpm_zsandpaper_adjust = 15;
uint8_t microsteps_zsandpaper = 16;
int steps_zsandpaper_subject = 639;
int steps_zsandpaper_docking = 80;

float rpm_plate = 40;
float rpm_sandpaper = 180;
boolean UseLowerPlate = true;

int pellets = 24;
int trials = 0;
int trials_correct = 0;
unsigned long duration = 3600000;
unsigned long time_begin;
boolean TimeOut = false;

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Adafruit_MotorShield AFMStop(0x61); // Rightmost jumper closed
Adafruit_MotorShield AFMSbottom(0x60); // Default address, no jumpers
Adafruit_StepperMotor *StepperMotor_Plate_Upper = AFMSbottom.getStepper(200, 1);
Adafruit_StepperMotor *StepperMotor_Plate_Lower = AFMSbottom.getStepper(200, 2);
Adafruit_StepperMotor *StepperMotor_Sandpaper = AFMStop.getStepper(200, 1);

void setup() {
  AFMStop.begin();
  StepperMotor_Sandpaper->setSpeed(rpm_sandpaper);
  StepperMotor_Sandpaper->step(1, BACKWARD, MICROSTEP);
  StepperMotor_Sandpaper->step(1, FORWARD, MICROSTEP);

  AFMSbottom.begin();
  StepperMotor_Plate_Upper->setSpeed(rpm_plate);
  StepperMotor_Plate_Upper->step(1, BACKWARD, MICROSTEP);
  StepperMotor_Plate_Upper->step(1, FORWARD, MICROSTEP);
  StepperMotor_Plate_Lower->setSpeed(rpm_plate);
  StepperMotor_Plate_Lower->step(1, BACKWARD, MICROSTEP);
  StepperMotor_Plate_Lower->step(1, FORWARD, MICROSTEP);

  pinMode(DIR_Zsandpaper, OUTPUT);
  pinMode(STEP_Zsandpaper, OUTPUT);
  digitalWrite(STEP_Zsandpaper, LOW);
  pinMode(LimitSwitch_Zsandpaper, INPUT_PULLUP);

  pinMode(DIR_ObjectPlate, OUTPUT);
  pinMode(STEP_ObjectPlate, OUTPUT);
  digitalWrite(STEP_ObjectPlate, LOW);

  pinMode(DIR_Y, OUTPUT);
  pinMode(STEP_Y, OUTPUT);
  digitalWrite(STEP_Y, LOW);
  pinMode(DIR_Z, OUTPUT);
  pinMode(STEP_Z, OUTPUT);
  digitalWrite(STEP_Z, LOW);
  pinMode(LimitSwitch_Y, INPUT_PULLUP);
  pinMode(LimitSwitch_Z, INPUT_PULLUP);

  pinMode(LeverL, INPUT_PULLUP);
  pinMode(LeverR, INPUT_PULLUP);

  pinMode(PiezoBuzzer, OUTPUT);

  pinMode(WaterValve, OUTPUT);

  pinMode(LaserTrigger, OUTPUT);

  pinMode(ACpowerTrigger, OUTPUT);

  pinMode(TrialStartTrigger, OUTPUT);

  Serial.begin(9600);

  Entropy.initialize();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  uint8_t buttons;
  boolean Adjustment = false;
  Adjustment = user_input(String("Make some adjust"), String("ment? No"), String("ment? Yes"), Adjustment);

  digitalWrite(ACpowerTrigger, HIGH);

  //adjust the position of the plate for switching objects
  if (Adjustment) {
    boolean ObjectPlateAdjust = false;
    ObjectPlateAdjust = user_input(String("Adjust Object Pl"), String("ate? No"), String("ate? Yes"), ObjectPlateAdjust);
    if (ObjectPlateAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Move obj plate");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        if ((buttons & BUTTON_RIGHT)) {
          stepxyz('O', 1, accduration_objectplate, rpm_objectplate_adjust, rpm_objectplate_adjust, 1);
        }
        if ((buttons & BUTTON_LEFT)) {
          stepxyz('O', 1, accduration_objectplate, rpm_objectplate_adjust, rpm_objectplate_adjust, 0);
        }
      }
    }
  }

  //adjust the angle of the sandpaper
  if (Adjustment) {
    boolean SPAngleAdjust = false;
    SPAngleAdjust = user_input(String("Adjust Sandpaper"), String("Angle? No"), String("Angle? Yes"), SPAngleAdjust);
    if (SPAngleAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Move sandpaper");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        while (buttons == lcd.readButtons()) {
        }
        if ((buttons & BUTTON_RIGHT)) {
          StepperMotor_Sandpaper->step(1, BACKWARD, DOUBLE);
        }
        if ((buttons & BUTTON_LEFT)) {
          StepperMotor_Sandpaper->step(1, FORWARD, DOUBLE);
        }
      }
    }
  }

  //homing Z sandpaper
  while (digitalRead(LimitSwitch_Zsandpaper) == HIGH) {
    stepxyz('S', microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper, rpm_zsandpaper, 0);
  }
  while (digitalRead(LimitSwitch_Zsandpaper) == LOW) {
    stepxyz('S', microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_homing, rpm_zsandpaper_homing, 1);
  }
  while (digitalRead(LimitSwitch_Zsandpaper) == HIGH) {
    stepxyz('S', microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_homing, rpm_zsandpaper_homing, 0);
  }
  //adjust the Z distance of the sandpaper to the mouse
  if (Adjustment) {
    boolean AdjustSandpaperZPosition = false;
    AdjustSandpaperZPosition = user_input(String("Adjust Sandpaper"), String("Z Position? No"), String("Z Position? Yes"), AdjustSandpaperZPosition);
    if (AdjustSandpaperZPosition) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      steps_zsandpaper_subject = 0;
      lcd.clear();
      lcd.print("Move Z of SP");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        if (buttons & BUTTON_RIGHT) {
          steps_zsandpaper_subject++;
          stepxyz('S', microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_adjust, rpm_zsandpaper_adjust, 1);
        }
        if (buttons & BUTTON_LEFT) {
          steps_zsandpaper_subject--;
          if (steps_zsandpaper_subject < 0) {
            steps_zsandpaper_subject = 0;
          }
          else {
            stepxyz('S', microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_adjust, rpm_zsandpaper_adjust, 0);
          }
        }
      }
      lcd.clear();
      lcd.print("Z of SP = ");
      lcd.print(steps_zsandpaper_subject);
      delay(2000);
      stepxyz('S', steps_zsandpaper_subject * microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper, rpm_zsandpaper, 0);
      while (digitalRead(LimitSwitch_Zsandpaper) == HIGH) {
        stepxyz('S', microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_homing, rpm_zsandpaper_homing, 0);
      }
    }
  }

  //homing Z axis
  while (digitalRead(LimitSwitch_Z) == HIGH) {
    stepxyz('Z', microsteps_z, accduration_z, rpm_z / 2, rpm_z / 2, 1);
  }
  while (digitalRead(LimitSwitch_Z) == LOW) {
    stepxyz('Z', microsteps_z, accduration_z, rpm_z_homing, rpm_z_homing, 0);
  }
  while (digitalRead(LimitSwitch_Z) == HIGH) {
    stepxyz('Z', microsteps_z, accduration_z, rpm_z_homing, rpm_z_homing, 1);
  }

  //homing Y axis
  while (digitalRead(LimitSwitch_Y) == HIGH) {
    stepxyz('Y', microsteps_y, accduration_y, rpm_y / 2, rpm_y / 2, 1);
  }
  while (digitalRead(LimitSwitch_Y) == LOW) {
    stepxyz('Y', microsteps_y, accduration_y, rpm_y_homing, rpm_y_homing, 0);
  }
  while (digitalRead(LimitSwitch_Y) == HIGH) {
    stepxyz('Y', microsteps_y, accduration_y, rpm_y_homing, rpm_y_homing, 1);
  }

  //adjust the time of water valve open
  if (Adjustment) {
    boolean AdjustValveOpenTime = false;
    AdjustValveOpenTime = user_input(String("Adjust valve ope"), String("n time? No"), String("n time? Yes"), AdjustValveOpenTime);
    if (AdjustValveOpenTime) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Close");
      lcd.setCursor(0, 1);
      lcd.print("Time = ");
      lcd.print(time_valve_open);
      lcd.print(" ms");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        while (buttons == lcd.readButtons()) {
        }
        if ((buttons & BUTTON_RIGHT)) {
          lcd.clear();
          lcd.print("Open");
          lcd.setCursor(0, 1);
          lcd.print("For ");
          lcd.print(time_valve_open);
          lcd.print(" ms");
          digitalWrite(WaterValve, HIGH);
          delay(time_valve_open);
          digitalWrite(WaterValve, LOW);
          lcd.clear();
          lcd.print("Close");
          lcd.setCursor(0, 1);
          lcd.print("Time = ");
          lcd.print(time_valve_open);
          lcd.print(" ms");
        }
        if ((buttons & BUTTON_DOWN)) {
          time_valve_open--;
          if (time_valve_open <= 0) {
            time_valve_open = 1;
          }
          lcd.clear();
          lcd.print("Close");
          lcd.setCursor(0, 1);
          lcd.print("Time = ");
          lcd.print(time_valve_open);
          lcd.print(" ms");
        }
        if ((buttons & BUTTON_UP)) {
          time_valve_open++;
          lcd.setCursor(0, 1);
          lcd.print("Time = ");
          lcd.print(time_valve_open);
          lcd.print(" ms");
        }
      }
    }
  }

  //adjust food loading position of Y and Z axes
  if (Adjustment) {
    boolean AdjustLoadPosition = false;
    AdjustLoadPosition = user_input(String("Adjust Load Posi"), String("tion? No"), String("tion? Yes"), AdjustLoadPosition);
    if (AdjustLoadPosition) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      steps_y_load = 0;
      steps_z_load = 0;
      lcd.clear();
      lcd.print("Move Y & Z axes");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        if (buttons & BUTTON_RIGHT) {
          steps_y_load++;
          stepxyz('Y', microsteps_y, accduration_y, rpm_y_adjust, rpm_y_adjust, 0);
        }
        if (buttons & BUTTON_LEFT) {
          steps_y_load--;
          if (steps_y_load < 0) {
            steps_y_load = 0;
          }
          else {
            stepxyz('Y', microsteps_y, accduration_y, rpm_y_adjust, rpm_y_adjust, 1);
          }
        }
        if (buttons & BUTTON_UP) {
          steps_z_load++;
          stepxyz('Z', microsteps_z, accduration_z, rpm_z_adjust, rpm_z_adjust, 0);
        }
        if (buttons & BUTTON_DOWN) {
          steps_z_load--;
          if (steps_z_load < 0) {
            steps_z_load = 0;
          }
          else {
            stepxyz('Z', microsteps_z, accduration_z, rpm_z_adjust, rpm_z_adjust, 1);
          }
        }
      }
      lcd.clear();
      lcd.print("Y load = ");
      lcd.print(steps_y_load);
      lcd.setCursor(0, 1);
      lcd.print("Z load = ");
      lcd.print(steps_z_load);
      delay(2000);
      stepxyz('Y', steps_y_load * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
      while (digitalRead(LimitSwitch_Y) == HIGH) {
        stepxyz('Y', microsteps_y, accduration_y, rpm_y_homing, rpm_y_homing, 1);
      }
      stepxyz('Z', steps_z_load * microsteps_z, accduration_z, rpm_z, rpm_z, 1);
      while (digitalRead(LimitSwitch_Z) == HIGH) {
        stepxyz('Z', microsteps_z, accduration_z, rpm_z_homing, rpm_z_homing, 1);
      }
    }
  }

  //adjust the Y position for switching objects
  if (Adjustment) {
    boolean AdjustDockingPosition = false;
    AdjustDockingPosition = user_input(String("Adjust Docking P"), String("osition? No"), String("osition? Yes"), AdjustDockingPosition);
    if (AdjustDockingPosition) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      steps_y_docking = 0;
      int steps_objectplate = 0;
      lcd.clear();
      lcd.print("Move Y axis!");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        if (buttons & BUTTON_RIGHT) {
          steps_y_docking++;
          stepxyz('Y', microsteps_y, accduration_y, rpm_y_adjust, rpm_y_adjust, 0);
        }
        if (buttons & BUTTON_LEFT) {
          steps_y_docking--;
          if (steps_y_docking < 0) {
            steps_y_docking = 0;
          }
          else {
            stepxyz('Y', microsteps_y, accduration_y, rpm_y_adjust, rpm_y_adjust, 1);
          }
        }

        if (buttons & BUTTON_UP) {
          stepxyz('O', 1, accduration_objectplate, rpm_objectplate_adjust, rpm_objectplate_adjust, 1);
          steps_objectplate++;
        }
        if (buttons & BUTTON_DOWN) {
          stepxyz('O', 1, accduration_objectplate, rpm_objectplate_adjust, rpm_objectplate_adjust, 0);
          steps_objectplate--;
        }
      }
      lcd.clear();
      lcd.print("Y dock = ");
      lcd.print(steps_y_docking);
      delay(2000);
      if (steps_objectplate > 0) {
        stepxyz('O', steps_objectplate, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 0);
      }
      else if (steps_objectplate < 0) {
        stepxyz('O', -steps_objectplate, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 1);
      }
      stepxyz('Y', steps_y_docking * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
      while (digitalRead(LimitSwitch_Y) == HIGH) {
        stepxyz('Y', microsteps_y, accduration_y, rpm_y_homing, rpm_y_homing, 1);
      }
    }
  }

  //adjust object distance to the mouse
  if (Adjustment) {
    for (int i = 0; i < 3; i++) {
      boolean AdjustSubjectPosition = false;
      switch (i) {
        case 0:
          AdjustSubjectPosition = user_input(String("Adjust lever to "), String("mouse? No"), String("mouse? Yes"), AdjustSubjectPosition);
          break;
        case 1:
          AdjustSubjectPosition = user_input(String("Adjust food to m"), String("ouse? No"), String("ouse? Yes"), AdjustSubjectPosition);
          break;
        case 2:
          AdjustSubjectPosition = user_input(String("Adjust lickomete"), String("r to mouse? No"), String("r to mouse? Yes"), AdjustSubjectPosition);
          break;
      }
      if (AdjustSubjectPosition) {
        stepxyz('Y', steps_y_docking * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 0);
        while ((buttons & BUTTON_SELECT)) {
          buttons = lcd.readButtons();
        }
        switch (i) {
          case 0:
            break;
          case 1:
            stepxyz('O', 200 * microsteps_objectplate / 4, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 0);
            break;
          case 2:
            stepxyz('O', 200 * microsteps_objectplate / 2, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 0);
            break;
        }
        int steps_y_docking2subject = 0;
        int steps_z_subject = 0;
        lcd.clear();
        lcd.print("Move Y & Z axes");
        while (!(buttons & BUTTON_SELECT)) {
          buttons = lcd.readButtons();
          if (buttons & BUTTON_RIGHT) {
            steps_y_docking2subject++;
            stepxyz('Y', microsteps_y, accduration_y, rpm_y_adjust, rpm_y_adjust, 0);
          }
          if (buttons & BUTTON_LEFT) {
            steps_y_docking2subject--;
            if (steps_y_docking2subject < 0) {
              steps_y_docking2subject = 0;
            }
            else {
              stepxyz('Y', microsteps_y, accduration_y, rpm_y_adjust, rpm_y_adjust, 1);
            }
          }

          if (buttons & BUTTON_UP) {
            steps_z_subject++;
            stepxyz('Z', microsteps_z, accduration_z, rpm_z_adjust, rpm_z_adjust, 0);
          }
          if (buttons & BUTTON_DOWN) {
            steps_z_subject--;
            if (steps_z_subject < 0) {
              steps_z_subject = 0;
            }
            else {
              stepxyz('Z', microsteps_z, accduration_z, rpm_z_adjust, rpm_z_adjust, 1);
            }
          }
        }
        lcd.clear();
        lcd.print("Y subj = ");
        lcd.print(steps_y_docking2subject + steps_y_docking);
        lcd.setCursor(0, 1);
        lcd.print("Z subj = ");
        lcd.print(steps_z_subject);
        delay(2000);
        stepxyz('Y', steps_y_docking2subject * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
        stepxyz('Z', steps_z_subject * microsteps_z, accduration_z, rpm_z, rpm_z, 1);
        while (digitalRead(LimitSwitch_Z) == HIGH) {
          stepxyz('Z', microsteps_z, accduration_z, rpm_z_homing, rpm_z_homing, 1);
        }
        switch (i) {
          case 0:
            steps_y_lever2mouse = steps_y_docking + steps_y_docking2subject;
            steps_z_lever2mouse = steps_z_subject;
            break;
          case 1:
            stepxyz('O', 200 * microsteps_objectplate / 4, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 1);
            steps_y_food2mouse = steps_y_docking + steps_y_docking2subject;
            steps_z_food2mouse = steps_z_subject;
            break;
          case 2:
            stepxyz('O', 200 * microsteps_objectplate / 2, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 1);
            steps_y_lickometer2mouse = steps_y_docking + steps_y_docking2subject;
            steps_z_lickometer2mouse = steps_z_subject;
            break;
        }
        stepxyz('Y', steps_y_docking * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
        while (digitalRead(LimitSwitch_Y) == HIGH) {
          stepxyz('Y', microsteps_y, accduration_y, rpm_y_homing, rpm_y_homing, 1);
        }
      }
    }
  }

  pellets = pellets * 2;
  //adjust the position of the two food plates
  if (Adjustment) {
    UseLowerPlate = user_input(String("Use two food pla"), String("tes? Yes"), String("tes? No"), UseLowerPlate);
    if (! UseLowerPlate) {
      pellets = pellets / 2;
    }

    boolean UpperPlateAdjust = false;
    UpperPlateAdjust = user_input(String("Adjust Upper Pla"), String("te? No"), String("te? Yes"), UpperPlateAdjust);
    if (UpperPlateAdjust) {
      while ((buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
      }
      lcd.clear();
      lcd.print("Move upper plate");
      while (!(buttons & BUTTON_SELECT)) {
        buttons = lcd.readButtons();
        while (buttons == lcd.readButtons()) {
        }
        if ((buttons & BUTTON_RIGHT)) {
          StepperMotor_Plate_Upper->step(1, BACKWARD, MICROSTEP);
        }
        if ((buttons & BUTTON_LEFT)) {
          StepperMotor_Plate_Upper->step(1, FORWARD, MICROSTEP);
        }
      }
    }

    if (UseLowerPlate) {
      boolean LowerPlateAdjust = false;
      LowerPlateAdjust = user_input(String("Adjust Lower Pla"), String("te? No"), String("te? Yes"), LowerPlateAdjust);
      if (LowerPlateAdjust) {
        while ((buttons & BUTTON_SELECT)) {
          buttons = lcd.readButtons();
        }
        lcd.clear();
        lcd.print("Move lower plate");
        while (!(buttons & BUTTON_SELECT)) {
          buttons = lcd.readButtons();
          while (buttons == lcd.readButtons()) {
          }
          if ((buttons & BUTTON_RIGHT)) {
            StepperMotor_Plate_Lower->step(1, BACKWARD, MICROSTEP);
          }
          if ((buttons & BUTTON_LEFT)) {
            StepperMotor_Plate_Lower->step(1, FORWARD, MICROSTEP);
          }
        }
      }
    }
  }

  //move the Y and Z axes to loading position
  stepxyz('Z', steps_z_load * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 0);
  stepxyz('Y', steps_y_load * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 0);

  stepxyz('S', steps_zsandpaper_docking * microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_accstart, rpm_zsandpaper, 1);

  while ((buttons & BUTTON_SELECT)) {
    buttons = lcd.readButtons();
  }
  lcd.clear();
  lcd.print("Press SELECT to ");
  lcd.setCursor(0, 1);
  lcd.print("Start Session");
  while (!(buttons & BUTTON_SELECT)) {
    buttons = lcd.readButtons();
  }
  lcd.clear();
  lcd.print("Session Begins");
  lcd.setCursor(0 , 1);
  lcd.print("Success ");
  lcd.print(trials_correct);
  lcd.print("/");
  lcd.print(trials);

  if (!UseLowerPlate) {
    StepperMotor_Plate_Upper->step(200 / (pellets + 1), FORWARD, MICROSTEP);
  }
  else {
    if (trials < pellets / 2 - 1) {
      StepperMotor_Plate_Upper->step(200 / (pellets / 2 + 1), FORWARD, MICROSTEP);
    }
    else {
      StepperMotor_Plate_Lower->step(200 / (pellets / 2 + 1), FORWARD, MICROSTEP);
    }
  }
  delay(1000);

  //move object plate to docking position and point lever to subject
  stepxyz('Z', steps_z_food * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 1);
  stepxyz('Y', (steps_y_docking - steps_y_load) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 0);
  stepxyz('Z', (steps_z_lever2mouse - steps_z_load + steps_z_food) * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 0);

  //move sandpaper to middle position
  StepperMotor_Sandpaper->step(50, FORWARD, DOUBLE);
  delay(250);

  time_begin = millis();
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

void loop() {
  while (trials < pellets && millis() - time_begin < duration) {
    uint8_t random_byte = Entropy.random(2); // return a 0 or a 1
    for (uint8_t i = 0; i < 2 && trials < pellets && millis() - time_begin < duration; i++) {
      digitalWrite(TrialStartTrigger, HIGH);
      //initialize sandpaper
      if (i == random_byte) {
        StepperMotor_Sandpaper->step(50, BACKWARD, DOUBLE);
      }
      else {
        StepperMotor_Sandpaper->step(50, FORWARD, DOUBLE);
      }
      delay(250);

      //sensory stimulus presentation
      stepxyz('S', (steps_zsandpaper_subject - steps_zsandpaper_docking) * microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_accstart, rpm_zsandpaper, 1);
      delay(time_sandpaper_stay);

      //move lever to mouse for making decision
      stepxyz('Y', (steps_y_lever2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 0);
      boolean leverl_state = digitalRead(LeverL);
      boolean leverr_state = digitalRead(LeverR);
      while (leverl_state && leverr_state) {
        leverl_state = digitalRead(LeverL);
        leverr_state = digitalRead(LeverR);
      }

      if ((i == random_byte && !leverl_state && leverr_state) || (i != random_byte && leverl_state && !leverr_state)) {
        //correct response, rewards the mouse
        tone(PiezoBuzzer, 2000, 300);
        stepxyz('Y', (steps_y_lever2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 1);
        stepxyz('O', 200 * microsteps_objectplate / 4, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 0);
        if (steps_z_food2mouse > steps_z_lever2mouse) {
          stepxyz('Z', (steps_z_food2mouse - steps_z_lever2mouse) * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 0);
        }
        else if (steps_z_food2mouse < steps_z_lever2mouse) {
          stepxyz('Z', (steps_z_lever2mouse - steps_z_food2mouse) * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 1);
        }
        stepxyz('Y', (steps_y_food2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 0);
        delay(time_food_stay);

        stepxyz('Y', (steps_y_food2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
        stepxyz('Z', steps_z_food2mouse * microsteps_z, accduration_z, rpm_z, rpm_z, 1);
        delay(time_eating);
        //switch water to mouse
        stepxyz('O', 200 * microsteps_objectplate / 4, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 0);
        stepxyz('Z', steps_z_lickometer2mouse * microsteps_z, accduration_z, rpm_z, rpm_z, 0);
        stepxyz('Y', (steps_y_lickometer2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y, rpm_y, 0);
        digitalWrite(WaterValve, HIGH);
        delay(time_valve_open);
        digitalWrite(WaterValve, LOW);
        delay(time_water_stay);

        //go to loading position for the next trial
        stepxyz('Y', (steps_y_lickometer2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
        stepxyz('O', 200 * microsteps_objectplate / 2, accduration_objectplate, rpm_objectplate_accstart, rpm_objectplate, 1);
        stepxyz('Z', (steps_z_lickometer2mouse - steps_z_load) * microsteps_z, accduration_z, rpm_z, rpm_z, 1);
        stepxyz('Y', (steps_y_docking - steps_y_load) * microsteps_y, accduration_y, rpm_y, rpm_y, 1);
        if (!UseLowerPlate) {
          StepperMotor_Plate_Upper->step(200 / (pellets + 1), FORWARD, MICROSTEP);
        }
        else {
          if (trials_correct < pellets / 2 - 1) {
            StepperMotor_Plate_Upper->step(200 / (pellets / 2 + 1), FORWARD, MICROSTEP);
          }
          else {
            StepperMotor_Plate_Lower->step(200 / (pellets / 2 + 1), FORWARD, MICROSTEP);
          }
        }
        delay(1000);

        //move object plate to docking position and point lever to subject
        stepxyz('Z', steps_z_food * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 1);
        stepxyz('Y', (steps_y_docking - steps_y_load) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 0);
        stepxyz('Z', (steps_z_lever2mouse - steps_z_load + steps_z_food) * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 0);

        trials_correct++;
      }
      else {
        //incorrect response
        tone(PiezoBuzzer, 100, 300);
        stepxyz('Y', (steps_y_lever2mouse - steps_y_docking) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 1);
      }
      digitalWrite(TrialStartTrigger, LOW);
      stepxyz('S', (steps_zsandpaper_subject - steps_zsandpaper_docking) * microsteps_zsandpaper, accduration_zsandpaper, rpm_zsandpaper_accstart, rpm_zsandpaper, 0);
      //move sandpaper to middle position
      if (i == random_byte) {
        StepperMotor_Sandpaper->step(50, FORWARD, DOUBLE);
      }
      else {
        StepperMotor_Sandpaper->step(50, BACKWARD, DOUBLE);
      }
      delay(250);

      trials++;
      lcd.setCursor(8, 1);
      lcd.print(trials_correct);
      lcd.print("/");
      lcd.print(trials);
    }
  }

  if (!TimeOut) {
    //move stage to loading position
    stepxyz('Z', (steps_z_lever2mouse - steps_z_load) * microsteps_z, accduration_z, rpm_z_accstart, rpm_z, 1);
    stepxyz('Y', (steps_y_docking - steps_y_load) * microsteps_y, accduration_y, rpm_y_accstart, rpm_y, 1);
    StepperMotor_Sandpaper->step(50, BACKWARD, DOUBLE);

    lcd.clear();
    lcd.print("Session Finished");
    lcd.setCursor(0, 1);
    lcd.print(trials_correct);
    lcd.print("/");
    lcd.print(trials);
    lcd.print(" correct");

    tone(PiezoBuzzer, 310, 5000);
    digitalWrite(ACpowerTrigger, LOW);
  }
  TimeOut = true;
}

void stepxyz(char axis, unsigned long steps, float acceleration_duration, float speed_initial, float speed_final, boolean motion_direction) {
  float acceleration;
  int pulse_interval;
  float speed_tmp = speed_initial;
  int count = 0;
  int DIR;
  int STEP;
  uint8_t microsteps_tmp;
  switch (axis) {
    case 'S':
      DIR = DIR_Zsandpaper;
      STEP = STEP_Zsandpaper;
      microsteps_tmp = microsteps_zsandpaper;
      break;
    case 'O':
      DIR = DIR_ObjectPlate;
      STEP = STEP_ObjectPlate;
      microsteps_tmp = microsteps_objectplate;
      break;
    case 'Y':
      DIR = DIR_Y;
      STEP = STEP_Y;
      microsteps_tmp = microsteps_y;
      break;
    case 'Z':
      DIR = DIR_Z;
      STEP = STEP_Z;
      microsteps_tmp = microsteps_z;
      break;
  }
  if (motion_direction) {
    digitalWrite(DIR, LOW);
  }
  else {
    digitalWrite(DIR, HIGH);
  }
  acceleration = (speed_final - speed_initial) / acceleration_duration;
  while (speed_tmp < speed_final) {
    speed_tmp = acceleration * 1 / (speed_tmp / 60.0 * 200 * microsteps_tmp) + speed_tmp;
    count++;
  }

  switch (axis) {
    case 'S':
      if (steps <= count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTG |= 1 << PORTG5; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTG &= ~(1 << PORTG5); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
        }
      }
      else if (steps <= 2 * count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTG |= 1 << PORTG5; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTG &= ~(1 << PORTG5); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      else {
        for (int i = 0; i < steps; i++) {
          PORTG |= 1 << PORTG5; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTG &= ~(1 << PORTG5); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else if (i >= steps - count - 1) {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      break;
    case 'O':
      if (steps <= count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTC |= 1 << PORTC3; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTC &= ~(1 << PORTC3); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
        }
      }
      else if (steps <= 2 * count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTC |= 1 << PORTC3; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTC &= ~(1 << PORTC3); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      else {
        for (int i = 0; i < steps; i++) {
          PORTC |= 1 << PORTC3; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTC &= ~(1 << PORTC3); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else if (i >= steps - count - 1) {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      break;
    case 'Y':
      if (steps <= count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTA |= 1 << PORTA5; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTA &= ~(1 << PORTA5); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
        }
      }
      else if (steps <= 2 * count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTA |= 1 << PORTA5; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTA &= ~(1 << PORTA5); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      else {
        for (int i = 0; i < steps; i++) {
          PORTA |= 1 << PORTA5; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTA &= ~(1 << PORTA5); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else if (i >= steps - count - 1) {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      break;
    case 'Z':
      if (steps <= count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTC |= 1 << PORTC6; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTC &= ~(1 << PORTC6); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
        }
      }
      else if (steps <= 2 * count + 1) {
        for (int i = 0; i < steps; i++) {
          PORTC |= 1 << PORTC6; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTC &= ~(1 << PORTC6); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      else {
        for (int i = 0; i < steps; i++) {
          PORTC |= 1 << PORTC6; //digitalWrite(STEP, HIGH);
          delayMicroseconds(4);
          PORTC &= ~(1 << PORTC6); //digitalWrite(STEP, LOW);
          pulse_interval = int (pow(10, 6) / (speed_initial / 60.0 * 200 * microsteps_tmp) - 4);
          delayMicroseconds(pulse_interval); //accurate between 3-16383 microseconds
          if (i < count) {
            speed_initial = acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
          else if (i >= steps - count - 1) {
            speed_initial = -acceleration * 1 / (speed_initial / 60.0 * 200 * microsteps_tmp) + speed_initial;
          }
        }
      }
      break;
  }
}
