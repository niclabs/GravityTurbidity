#include <GravityTurbidity.h>

#define PIN A0

GravityTurbidity turb = GravityTurbidity();

void setup() {
  turb.setLowCalibrationPoint(845, 206.961);
  turb.setHighCalibrationPoint(360, 2117.526);
  turb.calibrate();
}

void loop() {
  int value = analogRead(PIN);
  double t = turb.getTurbidity(value);
}
