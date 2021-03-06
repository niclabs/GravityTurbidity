#ifndef GRAVITY_TURBIDITY_H
#define GRAVITY_TURBIDITY_H

#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <stdint.h>

#define DEVICE_MEM_OFFSET 8

class GravityTurbidity {
public:
    GravityTurbidity(int device=0);
    float getTurbidity(float A, float alpha, int analog);
    float getTurbidity(int analog);
    void setLowCalibrationPoint(int analog, float ntu);
    void setHighCalibrationPoint(int analog, float ntu);
    void calibrate(int *analog_values, float *ntu_values, int n,
                                       float *A, float *alpha);
    void calibrate(int *analog_values, float *ntu_values, int n);
    void calibrate(float *A, float *alpha);
    void calibrate();
    // HEX export/import
    bool exportCalibration();
    void importCalibration(uint32_t A_hex, uint32_t alpha_hex);
    // EEPROM load/save
    bool loadCalibration();
    bool saveCalibration();

private:
    int low_turbidity_analog, high_turbidity_analog;
    float low_turbidity_ntu, high_turbidity_ntu;
    float A, alpha;
    int mem_offset;

    void linear_fit_if(int *xs, float *ys, int n,
                                float *slope, float *intercept);
    void float_to_hex(float fvalue);
};

#endif // GRAVITY_TURBIDITY_H
