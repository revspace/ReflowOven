#ifndef _PID_H
#define _PID_H

#include <stdint.h>

void    pid_init(float Kp,   float Ki,    float Kd);
int16_t pid_step(float temp, int16_t min, int16_t max);

#endif
