BOARD_TAG    = pro328
MONITOR_PORT = /dev/ttyUSB0
ARDUINO_QUIET = 1
include /usr/share/arduino/Arduino.mk

hexclean:
	rm -f *.hex

hex:
	rm -f *.hex
	ln -s build-$(BOARD_TAG)/*.hex
