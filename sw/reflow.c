#define BAUD 9600
#define MYUBRR (uint16_t)((F_CPU)/((BAUD)*16UL))

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "pid.h"
#include "tc.h"
#include "serial.h"

// macros
#define HEATER_PIN      PD3
#define HEATER_PORT     PORTD
#define HEATER_DDR      DDRD

// controller constants
#define SETPOINT_PREHEAT    NUM2TEMP(120.0)
#define SETPOINT_REFLOW     NUM2TEMP(240.0)

#define PFACTOR         1.0
#define IFACTOR         1.0
#define DFACTOR         1.0

// update timer settings
#define UT_PRESCALER    1024
#define UT_INTERVAL     1000    // in ms
#define UT_COMPARE      (uint16_t)(((F_CPU/UT_PRESCALER)*UT_INTERVAL)/1000)

// control timer settings
#define CT_PRESCALER    64
#define CT_INTERVAL     1       // in ms
#define CT_COMPARE      (uint8_t)(((F_CPU/CT_PRESCALER)*CT_INTERVAL)/1000)

// constants in memory
const char frac_str[4][3] = { "00", "25", "50", "75" };     // fractional values for displaying fixed-point

// main program state
volatile struct _reflow_state {
    enum { RS_OFF = 0, RS_PREHEAT = 1, RS_REFLOW = 2, RS_COOLDOWN = 3 } stage;    // explicit numbering to make debug code shorter
    uint16_t setpoint;
    uint16_t time;

    int16_t window;
} reflow_state;

// output status to serial port
inline void serial_xmit_temp(const char *s, uint16_t T) {
    serial_xmit(s);
    serial_xmit_num(TEMP2INT(T));
    serial_xmit_char('.');
    serial_xmit(frac_str[TEMP2FRAC(T)]);
}

inline void status_update() {
    serial_xmit("t: ");         serial_xmit_num(reflow_state.time);
    serial_xmit_temp(", sp: ",  reflow_state.setpoint);
    serial_xmit_temp(", tc: ",  tc_data.tc_temp);
    serial_xmit_temp(", ref: ", tc_data.ref_temp);
    serial_xmit(", window: ");   serial_xmit_num(reflow_state.window);

    serial_xmit_char('\n');
}

// control temperature
inline void control_temperature() {
    // find the on-time for the current second using PID
    reflow_state.window = pid_step(tc_data.tc_temp, 0, UT_INTERVAL/CT_INTERVAL);

    if(reflow_state.window > 0) {
        HEATER_PORT |= _BV(HEATER_PIN); // turn on heater
        TIMSK0      |= 0b00000010;      // turn on control timer
    }
}

// update timer compare interrupt
ISR(TIMER1_COMPA_vect) {
    reflow_state.time++;    // keep track of time
    tc_read();              // sensor update
    control_temperature();  // control heating element
    status_update();        // send update over serial
}

// control timer compare interrupt
ISR(TIMER0_COMPA_vect) {
    reflow_state.window--;  // count down
    if(reflow_state.window <= 0) {
        HEATER_PORT &= _BV(HEATER_PIN); // turn off heater
        TIMSK0      &= 0b11111101;      // turn off control timer
    }
}

// entrypoint
int main() {
    // initialize misc
    cli();                              // turn off interrupts during init

    serial_init(MYUBRR);
    serial_xmit("initialising");

    HEATER_DDR  |=  _BV(HEATER_PIN);    // set heater pin to output
    HEATER_PORT &= ~_BV(HEATER_PIN);    // turn off heater

    tc_init();
    pid_init(PFACTOR, IFACTOR, DFACTOR);


    // set up program state
    serial_xmit(" state");
    reflow_state.stage = RS_OFF;
    reflow_state.setpoint = SETPOINT_REFLOW;
    reflow_state.time = 0;


    // set up interrupts
    serial_xmit(", interrupts");

    // timer 0: control timer, for controlling the heating element
    TCCR0A = 0b00000010;    // CTC mode
    TCCR0B = 0b00000011;    // CTC mode, prescaler = 64
    OCR0A  = CT_COMPARE;
    TIMSK0 = 0b00000000;    // no compare interrupts for now


    // timer 1: update timer, for determining new control values
    TCCR1A = 0b00000000;    // CTC mode
    TCCR1B = 0b00001101;    // CTC mode, prescaler = 1024
    OCR1A  = UT_COMPARE;
    TIMSK1 = 0b00000010;    // compare interrupt A only

    sei();      // turn on all interrupts


    // do nothing if no interrupts
    serial_xmit(", all set!\n");
    for(;;);
}
