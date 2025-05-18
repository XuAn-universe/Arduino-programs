#include <Firmata.h>

byte analogPin = 0;

void setup()
{
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);
  Firmata.begin(57600);
}

void loop() {
  Firmata.sendAnalog(analogPin, analogRead(analogPin));
}
