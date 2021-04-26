#include "GravityTurbidity.h"
#include <math.h> // exp, log

#ifdef ARDUINO
#include <EEPROM.h>
#else
#include <stdio.h>
#endif // ARDUINO


#define EEPROM_read(address, dest) { \
        	byte *_dest = (byte*)&(dest); \
        	for(int i=0; i < sizeof(dest); i++) { \
        		_dest[i]=EEPROM.read(address+i);  \
        	} \
        } \

#define EEPROM_write(address, src) { \
        	byte *_src = (byte*)&(src); \
        	for(int i=0; i < sizeof(src); i++) {  \
        		EEPROM.update(address+i, _src[i]); \
        	} \
        }


// public

GravityTurbidity::GravityTurbidity(int device) {
    /* Will use 8 bytes in EEPROM (one DEVICE_MEM_OFFSET block):
     *  float A = EEPROM[mem_offset]
     *  float alpha = EEPROM[mem_offset+4]
     * */
    this->A = 0;
    this->alpha = 0;
    this->mem_offset = device*DEVICE_MEM_OFFSET;
    loadCalibration();
}

void GravityTurbidity::setLowCalibrationPoint(int analog, float ntu) {
    this->low_turbidity_analog = analog;
    this->low_turbidity_ntu = ntu;
}

void GravityTurbidity::setHighCalibrationPoint(int analog, float ntu) {
    this->high_turbidity_analog = analog;
    this->high_turbidity_ntu = ntu;
}

void GravityTurbidity::calibrate(int *analog_values, float *ntu_values, int n,
                                      float *A, float *alpha) {
    /* Inputs
     *  - analog_values: array of length n
     *  - ntu_values: array of length n
     *  - n
     * Outputs:
     *  - A
     *  - alpha
     *
     * Given a set of `n` samples
     *      (analog_value output by the sensor,
     *              known ntu_value when sampling analog_value)
     * it optimizes the function
     *      NTU(x: analog) = A*exp(alpha*x)
     * for the proper `A` and `alpha` values
     *  */

    // Make ntu_values its logarithm
    // MAKE SURE `ntu_values` IS NOT NEEDED AGAIN (or a copy was made)
    for (int i=0; i<n; i++) {
        ntu_values[i] = log(ntu_values[i]);
    }
    float slope, intercept;
    linear_fit_if(analog_values, ntu_values, n, &slope, &intercept);
    *A = exp(intercept);
    *alpha = slope;
}

void GravityTurbidity::calibrate(int *analog_values, float *ntu_values, int n) {
    // Calls `calibrate` storing the values to member A and alpha
    calibrate(analog_values, ntu_values, n, &(this->A), &(this->alpha));
}

void GravityTurbidity::calibrate(float *A, float *alpha) {
    // Calls `calibrate` reading sample values from
    // the two calibrations point already set.
    float ntu_values[] = {low_turbidity_ntu, high_turbidity_ntu};
    int analog_values[] = {low_turbidity_analog, high_turbidity_analog};
    calibrate(analog_values, ntu_values, 2, A, alpha);
}

void GravityTurbidity::calibrate() {
    calibrate(&(this->A), &(this->alpha));
}

float GravityTurbidity::getTurbidity(float A, float alpha, int analog) {
    // Returns a value in NTU
    return A*exp(alpha*analog);
}

float GravityTurbidity::getTurbidity(int analog) {
    return getTurbidity(A, alpha, analog);
}

bool GravityTurbidity::exportCalibration() {
    /* Exports calibration (this->A and this->alpha) to stdout.
     * A and alpha are 4-byte floats, printed as hexadecimal.
     *
     * Returns true if A or alpha are valid (non-zero) */

    if (this->A == 0 || this->alpha == 0) {
#ifdef ARDUINO
        Serial.println(F("[export] Calibration incomplete."));
#else
        printf("[export] Calibration incomplete.\n");
#endif // ARDUINO
        return 0;
    }

#ifdef ARDUINO
    Serial.print(F("A (float): "));
#else
    printf("A (float): ");
#endif // ARDUINO
    float_to_hex(this->A);

#ifdef ARDUINO
    Serial.print(F("alpha (float): "));
#else
    printf("alpha (float): ");
#endif // ARDUINO
    float_to_hex(this->alpha);

    return 1;
}

void GravityTurbidity::importCalibration(uint32_t A_hex, uint32_t alpha_hex) {
    this->A = *((float*)&A_hex);
    this->alpha = *((float*)&alpha_hex);
}

bool GravityTurbidity::loadCalibration() {
    /* Loads calibration (this->A and this->alpha) from EEPROM
     * Returns true if stored calibration is valid (non-zero) */
    float loaded_A, loaded_alpha;
    EEPROM_read(this->mem_offset, loaded_A);
    EEPROM_read(this->mem_offset+4, loaded_alpha);

    if ((*(uint32_t*)&loaded_A)==0xFFFFFFFF ||
        (*(uint32_t*)&loaded_alpha)==0xFFFFFFFF) {
        return 0;
    }
    this->A = loaded_A;
    this->alpha = loaded_alpha;
    return 1;
}

bool GravityTurbidity::saveCalibration() {
    /* Stores calibration (this->A and this->alpha) to EEPROM
     * Returns true if A or alpha are valid (non-zero) */
    if (this->A == 0 || this->alpha == 0) {
        return 0;
    }
    EEPROM_write(this->mem_offset, this->A);
    EEPROM_write(this->mem_offset+4, this->alpha);
    return 1;
}

// private

void GravityTurbidity::linear_fit_if(int *xs, float *ys, int n,
                            float *slope, float *intercept) {
    /* linear_fit for (i)nt xs and (f)loat ys
     * https://www.u-cursos.cl/usuario/125f006b1dcdd406c6c85298a6a14229/mi_blog/r/Apunte_Metodos_Experimentales.pdf
     * (p. 76)
     *
     * Inputs
     *  - xs: array of length n
     *  - ys: array of length n
     *  - n
     * Outputs:
     *  - slope
     *  - intercept
     *  */

    float sum_xy = 0;
    long sum_x = 0;
    long sum_x2 = 0;
    float sum_y = 0;
    for (int i=0; i<n; i++) {
        sum_xy += xs[i]*ys[i];
        sum_x += xs[i];
        sum_x2 += xs[i]*xs[i];
        sum_y += ys[i];
    }
    *slope = (sum_xy/sum_x - sum_y/n)/((float)sum_x2/sum_x - sum_x/n);
    *intercept = sum_y/n - (*slope)*sum_x/n;
}

void GravityTurbidity::float_to_hex(float fvalue) {
    uint32_t value = *(uint32_t*)&fvalue;
#ifdef ARDUINO
    Serial.print(F("0x"));
#else
    printf("0x");
#endif // ARDUINO

    // Only works for 4 bytes (float)
    for(int sh=24; sh>=0; sh-=8) {
        uint8_t hex = (value>>sh)&0xFF;
#ifdef ARDUINO
        if (hex < 0x10) {
            Serial.print('0');
        }
        Serial.print(hex, HEX);
#else
        printf("%02x", hex);
#endif // ARDUINO
    }
#ifdef ARDUINO
    Serial.println();
#else
    printf("\n");
#endif // ARDUINO
}

