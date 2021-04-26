#include <GravityTurbidity.h>

// EXECUTE FIRST WITH Save 1,
// AND THEN CHANGE THE MACRO TO Save 0

/* If Save = 0, this script will work in loading mode, reading values from
 * a previous calibration and using then to interpret turbidity.
 * If Save = 1, this script will work in loading mode, calculating calibration
 * parameters and saving them to EEPROM */
#define Save 1

#define PIN A0

GravityTurbidity turb = GravityTurbidity();


#if Save
void setup() { // Saving mode
  Serial.begin(9600);
  turb.setLowCalibrationPoint(845, 206.961);   // Calibration data to be exported: 206.961 NTU -> 845
  turb.setHighCalibrationPoint(360, 2117.526);  //                                2117.526 NTU -> 360
  turb.calibrate();
  Serial.println(F("Turbidity calibrated"));
}

void loop() {
  turb.saveCalibration();

  Serial.println(F("[infinite loop]"));
  while (1);
}

#else // Loading mode (interpreting values from loaded calibration)
void setup() {
  Serial.begin(9600);
  turb.loadCalibration();
}

void loop() {
  int value = analogRead(PIN);
  float t = turb.getTurbidity(value);
  Serial.println(t);
}
#endif // Save
