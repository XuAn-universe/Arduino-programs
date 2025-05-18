#include <Firmata.h>

byte DIPin = 4; // take the trigger using pin 4
byte DIPort = 0; // one port should have 8 pins

void outputPort(byte portNumber)
{
  Firmata.sendDigitalPort(portNumber, readPort(portNumber, 0xff));
}

void setPinModeCallback(byte pin, int mode) {
  if (IS_PIN_DIGITAL(pin)) {
    pinMode(PIN_TO_DIGITAL(pin), mode);
  }
}

void setup()
{
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);
  Firmata.attach(SET_PIN_MODE, setPinModeCallback);
  Firmata.begin(57600);
}

void loop()
{
  outputPort(DIPort);
}
