PROG    = fpga-384

LIBDIR=../../Base/pi
SOURCES = pi.cpp $(LIBDIR)/FPGA_library.cpp

CFLAGS += -g -Wall -Werror -I$(LIBDIR)
LKFLAGS=-lstdc++
uname_m := $(shell uname -m)
ifeq ($(uname_m), armv7l)
LKFLAGS+=-lwiringPi
endif

OBJECTS=$(patsubst %.cpp,%.o, $(SOURCES))

$(PROG) : $(OBJECTS)
	gcc $(notdir $(OBJECTS)) $(LKFLAGS) -o $@

%.o: %.cpp
	gcc $(CFLAGS) -c $< -o $(notdir $@)

.PHONY: clean
clean:
	rm -f $(PROG) *.o
