#ifndef _TC_H
#define _TC_H

extern struct _tc_data {
	uint16_t tc_temp;
	uint16_t ref_temp;
} tc_data;

void tc_init();
void tc_read();

#endif
