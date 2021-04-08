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
    // Make ntu_values its logarithm
    for (int i=0; i<n; i++) {
        ntu_values[i] = log(ntu_values[i]);
    }
    double slope, intercept;
    linear_fit_id(analog_values, ntu_values, n, &slope, &intercept);
    *A = exp(intercept);
    *alpha = slope;
}

void GravityTurbidity::calibrate(int *analog_values, double *ntu_values, int n) {
    calibrate(analog_values, ntu_values, n, &(this->A), &(this->alpha));
}

void GravityTurbidity::calibrate(double *A, double *alpha) {
    double ntu_values[] = {low_turbidity_ntu, high_turbidity_ntu};
    int analog_values[] = {low_turbidity_analog, high_turbidity_analog};
    calibrate(analog_values, ntu_values, 2, A, alpha);
}

void GravityTurbidity::calibrate() {
    calibrate(&(this->A), &(this->alpha));
}

double GravityTurbidity::getTurbidity(double A, double alpha, int analog) {
    return A*exp(alpha*analog);
}

double GravityTurbidity::getTurbidity(int analog) {
    return this->A*exp(this->alpha*analog);
}

// private

void GravityTurbidity::linear_fit_id(int *xs, double *ys, int n,
                            double *slope, double *intercept) {
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

