/* Copyright (c) 2018, Silicon Frog. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

This software is provided by the copyright holders and contributors "as is"
and any express or implied warranties, including, but not limited to, the
implied warranties of merchantability and fitness for a particular purpose
are disclaimed. In no event shall the copyright holder or contributors be
liable for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in
contract, strict liability, or tort (including negligence or otherwise)
arising in any way out of the use of this software, even if advised of the
possibility of such damage.  */

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include "FPGA_library.h"

int main(int argc, char *argv[]) {
    if (FPGAprogram((argc < 2) ? "fpga.bin" : argv[1])) {
        std::cout << "Programmed\n";
    } else {
        std::cout << "Failed\n";
    }

#if 0
    /* This section does not work. There is no output on the SPI pins,
     * even after resetting them. After a reboot, the SPI pins do change,
     * but programming the FPGA causes this to fail. For information:
     * the FPGA must be programmed in slave mode, as it is a master to
     * communicate with the flash, so GPIO is needed for this.
     */
    if (wiringPiSetup() < 0) {
        std::cerr << "Wiring Pi failed to set up\n";
    } else {
        pinMode(PIN_SCLK, 0b100);   // Reset SPI pins used in programming FPGA
        pinMode(PIN_MOSI, 0b100);   pinMode(PIN_MISO, 0b100);
        pinMode(PIN_CE0,  0b100);   pinMode(PIN_CE1,  0b100);

        const int CHANNEL = 0;
        if (wiringPiSPISetup(CHANNEL, 500000) < 0) {
            std::cerr << "Error: SPI #" << CHANNEL << ": " << strerror(errno) << "\n";
        } else {
            for (int loops = 0; loops < 16; loops++) {
                unsigned char buffer = 0x55 + loops;
                std::cout << "Tx: " << std::hex << (int)buffer << "    ";
                wiringPiSPIDataRW(CHANNEL, &buffer, 1);
                std::cout << "Rx: " << std::hex << (int)buffer << "\n";
                delay(1);
            }
        }
    }
#endif

    // Bit-bash the SPI data, as the max rate is ~ 0.5 MHz
    pinMode(PIN_SCLK, OUTPUT);      digitalWrite(PIN_SCLK, LOW);
    pinMode(PIN_MOSI, OUTPUT);      digitalWrite(PIN_MOSI, LOW);
    pinMode(PIN_MISO, INPUT);
    pinMode(PIN_CE0,  OUTPUT);      digitalWrite(PIN_CE0, HIGH);

    digitalWrite(PIN_CE0, LOW);
    for (int loops = 0; loops < 16; loops++) {
        unsigned char spiData = (unsigned char)(loops + 0x55);
        unsigned char spiRead = 0;

        unsigned char bit = 0x80;
        while (bit != 0) {
            if (digitalRead(PIN_MISO) == HIGH) {
                spiRead |= bit;
            }

            if (spiData & bit) {
                digitalWrite(PIN_MOSI, HIGH);
            } else {
                digitalWrite(PIN_MOSI, LOW);
            }
            // SPI data transfer at max ~ 0.5 MHz
            for (volatile int n = 0; n < 50; n++) { }
            digitalWrite(PIN_SCLK, HIGH);
            for (volatile int n = 0; n < 50; n++) { }
            digitalWrite(PIN_SCLK, LOW);

            bit >>= 1;
        }
        digitalWrite(PIN_MOSI, LOW);
        std::cout << "Tx: " << std::hex << (int)spiData << "    ";
        std::cout << "Rx: " << std::hex << (int)spiRead << "    ";
        if (spiRead == (spiData-1)) {
            std::cout << "OK";
        }
        std::cout << "\n";
        delay(1);
    }
    digitalWrite(PIN_CE0, HIGH);

    return 0;
}
