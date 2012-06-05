#define BAUD 9600
#define MYUBRR (uint16_t)((F_CPU)/((BAUD)*16UL))

#include <util/delay.h>

#include "tc.h"
#include "serial.h"


int main() {
	serial_init(MYUBRR);
	serial_xmit("initialising...\n");
	tc_init();
	serial_xmit_char('.');
	for(;;) {
		serial_xmit_char('.');
		tc_read();
		serial_xmit("tc: ");
		serial_xmit_num(tc_data.tc_temp);
		serial_xmit(", ref: ");
		serial_xmit_num(tc_data.ref_temp);
		serial_xmit_char('\n');
		_delay_ms(1000.0);
	}
}
