# Copyright (c) 2018, Silicon Frog. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# This software is provided by the copyright holders and contributors "as is"
# and any express or implied warranties, including, but not limited to, the
# implied warranties of merchantability and fitness for a particular purpose
# are disclaimed. In no event shall the copyright holder or contributors be
# liable for any direct, indirect, incidental, special, exemplary, or
# consequential damages (including, but not limited to, procurement of
# substitute goods or services; loss of use, data, or profits; or business
# interruption) however caused and on any theory of liability, whether in
# contract, strict liability, or tort (including negligence or otherwise)
# arising in any way out of the use of this software, even if advised of the
# possibility of such damage.

CC = g++
AR = ar
RM = rm
MKDIR = mkdir
# Set PRINT to nothing to suppress build messages
PRINT=echo

FPGATOOLS=/usr/local

ISPI=0
uname_m := $(shell uname -m)
ifeq ($(uname_m), armv7l)
ISPI=1
endif

ARD=arduino/FPGA$(PROJECT)
ARD_BIN=$(ARD)/arduino.hex
ARD_FBINH=fpgabin.h

ARD_FPGAPROG=fpgaprog
ARD_FPGAPROGSRC=../Base/arduino/$(ARD_FPGAPROG).cpp

PI=pi
PI_SOURCE=$(PI)/pi.cpp
PI_PROG=fpga-384
PI_BIN=$(PI)/$(PI_PROG)

FPATH=$(FPGATOOLS)/bin
FSHARE=$(FPGATOOLS)/share

YOSYS=$(FPATH)/yosys
ARACHNE=$(FPATH)/arachne-pnr
PACK=$(FPATH)/icepack

YOSYS_FLAGS=-q -p
ARACHNE_FLAGS=-q -d 384 -P qn32
PACK_FLAGS=

BLIF=fpga.blif
FPGATXT=fpga.txt
FPGABIN=fpga.bin
CHIP_BIN=$(FSHARE)/arachne-pnr/chipdb-384.bin

FPGA_SRC=$(PROJECT).v
PCF=./ice40-384_$(PROJECT).pcf

ARD_SOURCE=$(ARD)/FPGA$(PROJECT).ino

ifeq ($(ISPI), 1)
all: fpga pi
else
all: fpga $(ARD)/$(ARD_FBINH) arduino
endif

fpga: $(FPGABIN)

.PHONY: fpga pi arduino upload clean help

help:
	echo "\nMake targets:\n"
	echo "all              - [Default] Generates fpga.bin and required upload files"
	echo "                     Also includes 'make pi' or 'make arduino' depending upon system"
	echo "                     Generally, on a pi: make && ./sf_iceprog fpga.bin"
	echo "                     And for an Arduino: make && make upload (if Arduino-mk installed)"
	echo ""
	echo "clean            - Remove all generated files"
	echo ""
	echo "pi               - Makes sf_iceprog. Used to upload an fpga.bin"
	echo ""
	echo "arduino          - Creates all necessary project files for arduino platform"
	echo "upload           - Upload files to the arduino (if Arduino.mk installed)"
	echo "\n"

$(FPGABIN): $(FPGA_SRC)
	$(YOSYS) $(YOSYS_FLAGS) "read_verilog $(FPGA_SRC); synth_ice40 -blif fpga.blif"
	$(ARACHNE) $(ARACHNE_FLAGS) -c $(CHIP_BIN) -p $(PCF) -o $(FPGATXT) $(BLIF)
	$(PACK) $(FPGATXT) $(FPGABIN)
	@$(PRINT) "\n*** $(FPGABIN) generated, ready for upload to FPGA\n"

pi: fpga $(PI_BIN)

$(PI_BIN): $(PI_SOURCE)
	cd $(PI) && make
	ln -f -s $(PI_BIN)
	@$(PRINT) "\n*** Built $(PI_BIN).  Use: \"./$(PI_PROG) $(FPGABIN)\" to upload\n"

arduino: fpga $(ARD_BIN)

$(ARD_BIN): $(ARD_SOURCE) $(ARD)/$(ARD_FBINH) $(ARD_FPGAPROG)
	cd $(ARD) && make && make hex
	@$(PRINT) "\n*** HEX file built - ready for make upload\n"

$(ARD_FPGAPROG): $(ARD_FPGAPROGSRC)
	cd $(ARD) && $(CC) -Wall -Werror -lstdc++ ../../$< -o $@

$(ARD)/$(ARD_FBINH): $(ARD_FPGAPROG) $(FPGABIN)
	cd $(ARD) && ./$(ARD_FPGAPROG) ../../$(FPGABIN) >$(ARD_FBINH)

upload: all
	cd $(ARD) && make upload && $(PRINT) "\n*** Arduino programmed\n"

clean:
	$(RM) -f $(FPGATXT) $(FPGABIN) $(BLIF) $(PROJECT).vcd
ifeq ($(ISPI), 1)
	cd $(PI) && make clean >/dev/null
	$(RM) -f $(PI_PROG)
else
	cd $(ARD) && make hexclean >/dev/null && make clean >/dev/null && $(RM) -f $(ARD_FBINH) $(ARD_FPGAPROG)
endif
