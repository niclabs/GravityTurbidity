#include "GravityTurbidity.h"
#include <math.h> // exp, log

// public

GravityTurbidity::GravityTurbidity() {
    this->A = 0;
    this->alpha = 0;
}

void GravityTurbidity::setLowCalibrationPoint(int analog, double ntu) {
    this->low_turbidity_analog = analog;
    this->low_turbidity_ntu = ntu;
}

void GravityTurbidity::setHighCalibrationPoint(int analog, double ntu) {
    this->high_turbidity_analog = analog;
    this->high_turbidity_ntu = ntu;
}

void GravityTurbidity::calibrate(int *analog_values, double *ntu_values, int n,
                                      double *A, double *alpha) {
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
    double slope, intercept;
    linear_fit_id(analog_values, ntu_values, n, &slope, &intercept);
    *A = exp(intercept);
    *alpha = slope;
}

void GravityTurbidity::calibrate(int *analog_values, double *ntu_values, int n) {
    // Calls `calibrate` storing the values to member A and alpha
    calibrate(analog_values, ntu_values, n, &(this->A), &(this->alpha));
}

void GravityTurbidity::calibrate(double *A, double *alpha) {
    // Calls `calibrate` reading sample values from
    // the two calibrations point already set.
    double ntu_values[] = {low_turbidity_ntu, high_turbidity_ntu};
    int analog_values[] = {low_turbidity_analog, high_turbidity_analog};
    calibrate(analog_values, ntu_values, 2, A, alpha);
}

void GravityTurbidity::calibrate() {
    calibrate(&(this->A), &(this->alpha));
}

double GravityTurbidity::getTurbidity(double A, double alpha, int analog) {
    // Returns a value in NTU
    return A*exp(alpha*analog);
}

double GravityTurbidity::getTurbidity(int analog) {
    return getTurbidity(A, alpha, analog);
}

bool GravityTurbidity::export_calibration() {
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

// private

void GravityTurbidity::linear_fit_id(int *xs, double *ys, int n,
                            double *slope, double *intercept) {
    /* linear_fit for (i)nt xs and (d)ouble ys
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

    double sum_xy = 0;
    long sum_x = 0;
    long sum_x2 = 0;
    double sum_y = 0;
    for (int i=0; i<n; i++) {
        sum_xy += xs[i]*ys[i];
        sum_x += xs[i];
        sum_x2 += xs[i]*xs[i];
        sum_y += ys[i];
    }
    *slope = (sum_xy/sum_x - sum_y/n)/((double)sum_x2/sum_x - sum_x/n);
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
        Serial.print(hex < 0x10 ? '0' : '');
        Serial.print(hex);
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

