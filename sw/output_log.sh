#!/bin/sh
mkdir -p log
stty -F /dev/ttyUSB0 9600 cs8 -clocal -crtscts -parenb
cat /dev/ttyUSB0 | tee log/`date +%Y-%m-%d_%H:%M:%S`.log
