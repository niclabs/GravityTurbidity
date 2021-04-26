#include <GravityTurbidity.h>

// EXECUTE FIRST WITH Export 1,
// take note of the export values
// write them on A_hex and alpha_hex
// AND THEN CHANGE THE MACRO TO Export 0

/* If Export = 0, this script will work in importing mode, reading values from
 * a previous calibration and using them.
 * If Export = 1, this script will work in exporting mode, calculating calibration
 * parameters and exporting them to the console */
#define Export 1

#define PIN A0

GravityTurbidity turb = GravityTurbidity();


#if Export
void setup() { // Exporting mode
  Serial.begin(9600);
  turb.setLowCalibrationPoint(845, 206.961);   // Calibration data to be exported: 206.961 NTU -> 845
  turb.setHighCalibrationPoint(360, 2117.526);  //                                2117.526 NTU -> 360
  turb.calibrate();

  Serial.println(F("Turbidity calibrated"));
}

void loop() {
  turb.exportCalibration();

  Serial.println(F("[infinite loop]"));
  while (1);
}

#else // Importing mode (interpreting values from imported calibration)

/* REPLACE THESE HEX VALUES WITH THE ONES
 * PRINTED BY THE exportCalibration STEP */
uint32_t A_hex = 0x00000000; // <- 0x0..0 is 0f (will make turbidity return always 0)
uint32_t alpha_hex = 0x19857725;

void setup() {
  Serial.begin(9600);
  turb.importCalibration(A_hex, alpha_hex); // imports floats as hex
}

void loop() {
  int value = analogRead(PIN);
  float t = turb.getTurbidity(value);
  Serial.println(t);
}
#endif // Export
