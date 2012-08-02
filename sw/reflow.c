#define BAUD 9600
#define MYUBRR (uint16_t)((F_CPU)/((BAUD)*16UL))

#include <avr/io.h>
#include <util/delay.h>

#include "tc.h"
#include "serial.h"

// macros
#define HEATER_PIN PD3
#define HEATER_PORT PORTD
#define HEATER_DDR DDRD

#define CONTROL_INTERVAL (1000.0)

#define SETPOINT_LOWER NUM2TEMP(100.0)
#define SETPOINT_UPPER NUM2TEMP(150.0)

// prescaler for control timer
#define CT_PRESCALER 1024
#define CT_COMPARE (uint16_t)(F_CPU/CT_PRESCALER)

// constants in memory
const char frac_str[4][3] = { "00", "25", "50", "75" };

// main program state
volatile struct _reflow_state {
	enum { RS_OFF = 0, RS_PREHEAT = 1, RS_REFLOW = 2, RS_COOLDOWN_1 = 3, RS_COOLDOWN_2 = 4 } stage;	// explicit numbering to make debug code shorter
	uint16_t setpoint;
	uint16_t time;
} reflow_state;

// output status to serial port
inline void status_update() {
	serial_xmit("t: ");
	serial_xmit_num(reflow_state.time);

	serial_xmit(", sp: ");
	serial_xmit_num(TEMP2INT(reflow_state.setpoint));

	serial_xmit(", state: ");
	serial_xmit_num(reflow_state.state);

	serial_xmit(", tc: ");
	serial_xmit_num(TEMP2INT(tc_data.tc_temp));
	serial_xmit_char('.');
	serial_xmit(frac_str[TEMP2FRAC(tc_data.tc_temp)]);

	serial_xmit(", ref: ");
	serial_xmit_num(TEMP2INT(tc_data.ref_temp));
	serial_xmit_char('.');
	serial_xmit(frac_str[TEMP2FRAC(tc_data.ref_temp)]);

	serial_xmit_char('\n');
}

// control temperature
inline void control_temperature() {
	// TODO
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
