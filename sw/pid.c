#include "tc.h"
#include "serial.h"
#include "pid.h"

static float Kp, Ki, Kd;
static float Imin, Imax;
static float setpoint;

void pid_init(float p, float i, float d, float i_min, float i_max) {
    Kp = p;
    Ki = i;
    Kd = d;

    Imax = i_max;
    Imin = i_min;
}

void pid_change_setpoint(float new_setpoint) {
    setpoint = new_setpoint;
}

int16_t pid_step(float delta, float input, int16_t o_min, int16_t o_max) {
    static float last_error, acc_error;

    // scale integral & differential factors with sample time
    float ki = Ki * delta;
    float kd = Kd / delta;

    // compute error, derivative of error
    float error = setpoint - input;
    float delta_error = error - last_error;

    // accumulate and clip integral term
    acc_error += ki * error;
    if(acc_error > Imax) acc_error = Imax;
    if(acc_error < Imin) acc_error = Imin;

    // compute output
    float output = Kp * error + acc_error - kd * delta_error;

    serial_xmit("\nerror: "); serial_xmit_num((int16_t)(error));
    serial_xmit("\nlast: "); serial_xmit_num((int16_t)(last_error));
    serial_xmit("\nacc: "); serial_xmit_num((int16_t)(acc_error));
    serial_xmit("\nout: "); serial_xmit_num((int16_t)(output));
    serial_xmit("\n");

    last_error = error; // store last error for differential term

    return (output>o_max) ? o_max : ((output<o_min)?o_min:(int16_t)output);
}
