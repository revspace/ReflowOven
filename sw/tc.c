#include <avr/io.h>
#include <util/delay.h>

#include "tc.h"

struct _tc_data tc_data;

void tc_init() {
	ADMUX	= 0b01000000;	// ref is AVCC, select ADC0
	ADCSRA	= 0b10000111;	// ADC on, 128x prescaler
	ADCSRB	= 0b00000000;
	DIDR0	= 0b00000011;	// disable ADC0 and 1 as digital in
}

void tc_read() {
	ADMUX	&= 0b11110000;			// clear last 4 bits == select ADC0
	ADCSRA	|= 0b11000000;			// start conversion
	while(ADCSRA & 0b01000000);		// wait for conversion
	tc_data.tc_temp  =  ADCL;		// store ADC value -- read ADCL first
	tc_data.tc_temp |= (ADCH << 8);	// read in ADCH
}
