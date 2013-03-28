#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

void serial_init(const uint16_t ubrr);
void serial_xmit_char(const char c);
void serial_xmit(const char *s);
void serial_xmit_num(int16_t n);

#endif
