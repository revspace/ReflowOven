#include <avr/io.h>
#include <util/delay.h>

#include "tc.h"

#define TC_SCALE	0.42
#define REF_SCALE	0.4832
#define REF_OFFSET	175.

struct _tc_data tc_data;

void tc_init() {
	ADMUX	= 0b01000000;	// ref is AVCC, select ADC0
	ADCSRA	= 0b10000111;	// ADC on, 128x prescaler
	ADCSRB	= 0b00000000;
	DIDR0	= 0b00000011;	// disable ADC0 and 1 as digital in
}

void tc_read() {
	uint16_t v;

	ADMUX	= 0b01000001;			// ref from AVCC, input from adc1
	ADCSRA	= 0b11000111;			// start conversion
	while(ADCSRA & 0b01000000);		// wait for conversion
	v  =  ADCL;						// store ADC value -- read ADCL first
	v |= (ADCH << 8);				// read in ADCH
	tc_data.ref_temp = (uint16_t)((v*REF_SCALE - REF_OFFSET)*TEMPFACTOR);

	ADMUX	= 0b01000000;			// ref from AVCC, input from adc0
	ADCSRA	= 0b11000111;			// start conversion
	while(ADCSRA & 0b01000000);		// wait for conversion
	v  =  ADCL;						// store ADC value -- read ADCL first
	v |= (ADCH << 8);				// read in ADCH
	tc_data.tc_temp = ((uint16_t)(v*TC_SCALE*TEMPFACTOR)) + tc_data.ref_temp;
}
