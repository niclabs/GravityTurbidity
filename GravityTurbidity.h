#ifndef GRAVITY_TURBIDITY_H
#define GRAVITY_TURBIDITY_H

class GravityTurbidity {
public:
    GravityTurbidity();
    double getTurbidity(double A, double alpha, int analog);
    double getTurbidity(int analog);
    void setLowCalibrationPoint(int analog, double ntu);
    void setHighCalibrationPoint(int analog, double ntu);
    void calibrate(int *analog_values, double *ntu_values, int n,
                                       double *A, double *alpha);
    void calibrate(int *analog_values, double *ntu_values, int n);
    void calibrate(double *A, double *alpha);
    void calibrate();
    bool export_calibration();

private:
    int low_turbidity_analog, high_turbidity_analog;
    double low_turbidity_ntu, high_turbidity_ntu;
    double A, alpha;

    void linear_fit_id(int *xs, double *ys, int n,
                                double *slope, double *intercept);
    void float_to_hex(float fvalue);
};

#endif // GRAVITY_TURBIDITY_H
