#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

void serial_init(uint16_t ubrr);
void serial_xmit_char(char c);
void serial_xmit(char *s);
void serial_xmit_num(uint16_t n);

#endif
