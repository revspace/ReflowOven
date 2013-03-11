#ifndef _TC_H
#define _TC_H

#include <stdint.h>

#define TEMP2INT(T)  ((uint16_t)T)
#define TEMP2FRAC(T) (((uint16_t)(T*4))%4)

extern struct _tc_data {
	float tc_temp;
	float ref_temp;
} tc_data;

void tc_init();
void tc_read();

#endif
