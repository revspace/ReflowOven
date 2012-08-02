#ifndef _TC_H
#define _TC_H

#define TEMPFACTOR		(4.)
#define TEMP2INT(T)		((T)>>2)
#define TEMP2FRAC(T)	((T)&0x03)

#define NUM2TEMP(N)		(uint16_t)((N)*TEMPFACTOR)

extern struct _tc_data {
	uint16_t tc_temp;
	uint16_t ref_temp;
} tc_data;

void tc_init();
void tc_read();

#endif
