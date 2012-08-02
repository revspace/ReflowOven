#define BAUD 9600
#define MYUBRR (uint16_t)((F_CPU)/((BAUD)*16UL))

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "tc.h"
#include "serial.h"

// macros
#define HEATER_PIN PD3
#define HEATER_PORT PORTD
#define HEATER_DDR DDRD

// controller 
#define SETPOINT_PREHEAT	NUM2TEMP(120.0)
#define SETPOINT_REFLOW		NUM2TEMP(240.0)

#define PREHEAT_TIME		60
#define REFLOW_TIME			30

#define SLEW_RATE	NUM2TEMP(3)

#define PFACTOR		1.0
#define IFACTOR		1.0
#define DFACTOR		1.0

// prescaler for control timer
#define CT_PRESCALER		1024
#define CONTROL_INTERVAL	1000
#define CT_COMPARE			(uint16_t)(((F_CPU/CT_PRESCALER)*CONTROL_INTERVAL)/1000)

// temperature controller

// constants in memory
const char frac_str[4][3] = { "00", "25", "50", "75" };

// main program state
volatile struct _reflow_state {
	enum { RS_OFF = 0, RS_PREHEAT = 1, RS_REFLOW = 2, RS_COOLDOWN = 3 } stage;	// explicit numbering to make debug code shorter
	uint16_t setpoint;
	uint16_t time;

	int16_t prev_error;
	int16_t acc_error;
} reflow_state;

// output status to serial port
inline void serial_xmit_temp(const char *s, uint16_t T) {
	serial_xmit(s);
	serial_xmit_num(TEMP2INT(T));
	serial_xmit_char('.');
	serial_xmit(frac_str[TEMP2FRAC(T)]);
}

inline void status_update() {
	serial_xmit("t: ");			serial_xmit_num(reflow_state.time);
	serial_xmit(", state: ");	serial_xmit_num(reflow_state.stage);
	serial_xmit_temp(", sp: ", reflow_state.setpoint);
	serial_xmit_temp(", tc: ", tc_data.tc_temp);
	serial_xmit_temp(", ref: ", tc_data.ref_temp);
	serial_xmit_temp(", pe: ", reflow_state.prev_error);
	serial_xmit_temp(", ae: ", reflow_state.acc_error);

	serial_xmit_char('\n');
}

// control temperature
inline void control_temperature() {
	// determine new setpoint
	switch(reflow_state.stage) {
		case RS_OFF:
			reflow_state.setpoint = 0;
			break;

		case RS_PREHEAT:
			if(reflow_state.setpoint < SETPOINT_PREHEAT) reflow_state.setpoint += SLEW_RATE;
			else {
				reflow_state.setpoint = SETPOINT_PREHEAT;
				reflow_state.time++;
				if(reflow_state.time > PREHEAT_TIME) {
					reflow_state.time = 0;
					reflow_state.stage = RS_REFLOW;
				}
			}
			break;

		case RS_REFLOW:
			if(reflow_state.setpoint < SETPOINT_REFLOW) reflow_state.setpoint += SLEW_RATE;
				else {
				reflow_state.setpoint = SETPOINT_REFLOW;
				reflow_state.time++;
				if(reflow_state.time > REFLOW_TIME) {
					reflow_state.time = 0;
					reflow_state.stage = RS_COOLDOWN;
				}
			}

		case RS_COOLDOWN:
			if(reflow_state.setpoint >= SLEW_RATE) reflow_state.setpoint -= SLEW_RATE;
			else {
				reflow_state.setpoint = 0;
				reflow_state.stage = RS_OFF;
			}
	}

	// update error, delta-error, accumulated error
	int16_t error = reflow_state.setpoint - tc_data.tc_temp;
	int16_t derr  = error - reflow_state.prev_error;
	reflow_state.acc_error += error;
	reflow_state.prev_error = error;

	// control action
	int16_t plant = (int16_t)(error*PFACTOR + reflow_state.acc_error*IFACTOR + derr*DFACTOR);
	if(plant > 1)		HEATER_PORT |=  _BV(HEATER_PIN);
	else if(plant < -1)	HEATER_PORT &= ~_BV(HEATER_PIN);
}

// main control timer interrupt
ISR(TIMER1_COMPA_vect) {
	reflow_state.time++;
	tc_read();				// sensor update
	control_temperature();
	status_update();
}

// entrypoint
int main() {
	// initialize misc
	cli();
	serial_init(MYUBRR);
	serial_xmit("initialising");
	HEATER_DDR |= _BV(HEATER_PIN);
	tc_init();

	// set up program state
	serial_xmit(" state");
	reflow_state.stage = RS_OFF;
	reflow_state.setpoint = 0;
	reflow_state.time = 0;
	reflow_state.prev_error = 0;
	reflow_state.acc_error = 0;

	// set up interrupts
	serial_xmit(", interrupts");
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;	// CTC mode, prescaler = 1024
	OCR1A  = CT_COMPARE;
	TIMSK1 = 0b00000010;
	sei();	// turn on all interrupts

	// do nothing if no interrupts
	serial_xmit(", all set!\n");
	for(;;);
}
