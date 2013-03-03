CC		:= avr-gcc
CFLAGS	:= -Wall -Wextra -mmcu=atmega328p -O3 -DF_CPU=12000000UL
ADFLAGS	:= -p m328p -B 4 -c usbasp

all : reflow.hex

clean :
	rm -f *.hex *.elf *.o

program : reflow.hex
	avrdude $(ADFLAGS) -U flash:w:reflow.hex:i -U lfuse:w:0b11100110:m

reflow.hex : reflow.elf
	size $^
	avr-objcopy -j .text -j .data -O ihex $^ $@

reflow.elf : reflow.o serial.o tc.o pid.o
	$(CC) $(CFLAGS) -o $@ $^


reflow.o : reflow.c serial.h tc.h pid.h
	$(CC) $(CFLAGS) -c -o $@ $<

serial.o : serial.c serial.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
tc.o : tc.c tc.h
	$(CC) $(CFLAGS) -c -o $@ $<

pid.o : pid.c pid.h
	$(CC) $(CFLAGS) -c -o $@ $<
