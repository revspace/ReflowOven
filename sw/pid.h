#ifndef _PID_H
#define _PID_H

#include "tc.h"

void pid_init(float p, float i, float d, float i_min, float i_max);
void pid_change_setpoint(float new_setpoint);
int16_t pid_step(float delta, float input, int16_t o_min, int16_t o_max);

#endif
