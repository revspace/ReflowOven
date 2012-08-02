#define BAUD 9600
#define MYUBRR (uint16_t)((F_CPU)/((BAUD)*16UL))

#include <avr/io.h>
#include <util/delay.h>

#include "tc.h"
#include "serial.h"

#define HEATER_PIN PD3
#define HEATER_PORT PORTD
#define HEATER_DDR DDRD

#define CONTROL_INTERVAL (1000.0)

#define SETPOINT_LOWER NUM2TEMP(100.0)
#define SETPOINT_UPPER NUM2TEMP(150.0)

const char frac_str[4][3] = { "00", "25", "50", "75" };

int main() {
	HEATER_DDR |= _BV(HEATER_PIN);
	serial_init(MYUBRR);
	serial_xmit("initialising...\n");
	tc_init();
	serial_xmit_char('.');
	for(;;) {
		// sensor update
		tc_read();

		// temperature control
		if(tc_data.tc_temp <= SETPOINT_LOWER)
			HEATER_PORT |= _BV(HEATER_PIN);
		else if(tc_data.tc_temp >= SETPOINT_UPPER)
			HEATER_PORT &= ~_BV(HEATER_PIN);

		// status output
		serial_xmit("sp: ");
		serial_xmit_num(TEMP2INT(SETPOINT_LOWER));
		serial_xmit(", tc: ");
		serial_xmit_num(TEMP2INT(tc_data.tc_temp));
		serial_xmit_char('.');
		serial_xmit(frac_str[TEMP2FRAC(tc_data.tc_temp)]);

		serial_xmit(", ref: ");
		serial_xmit_num(TEMP2INT(tc_data.ref_temp));
		serial_xmit_char('.');
		serial_xmit(frac_str[TEMP2FRAC(tc_data.ref_temp)]);

		serial_xmit((HEATER_PORT&_BV(HEATER_PIN))?", heater on":", heater off");

		serial_xmit_char('\n');

		// delay
		_delay_ms(CONTROL_INTERVAL);
	}
}
