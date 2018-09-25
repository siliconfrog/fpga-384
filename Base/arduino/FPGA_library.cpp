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

#include <SPI.h>
#include "FPGA_library.h"

class SPIsim {
public:
    SPIsim(const uint8_t _pinClk = 13, const uint8_t _pinIn = 12, const uint8_t _pinOut = 11) :
            pinClk(_pinClk), pinIn(_pinIn), pinOut(_pinOut) {
        pinMode(pinClk, OUTPUT);
        pinMode(pinOut, OUTPUT);
        pinMode(pinIn, INPUT);

        digitalWrite(pinClk, LOW);
        digitalWrite(pinOut, LOW);
    }

    ~SPIsim() {}

    uint8_t transfer(const uint8_t data) {
        uint8_t read = 0;
        for (uint8_t i = 0; i < 8; i++) {
            uint8_t mask = 128 >> i;
            if ((data & mask) > 0) {
                digitalWrite(pinOut, HIGH);
            } else {
                digitalWrite(pinOut, LOW);
            }
            digitalWrite(pinClk, HIGH);
            asm volatile("nop");
            digitalWrite(pinClk, LOW);
            if (digitalRead(pinIn) == HIGH) {
                read |= mask;
            }
            asm volatile("nop");
        }
        return read;
    }

private:
    uint8_t pinClk;
    uint8_t pinIn;
    uint8_t pinOut;
};

bool FPGAprogram(const unsigned char *fpgaBin PROGMEM, const unsigned short int fpgaBinSize) {
    pinMode(PIN_CDONE, INPUT);
    pinMode(PIN_CRESET, OUTPUT);
    pinMode(PIN_SS, OUTPUT);
    digitalWrite(PIN_CRESET, HIGH);
    digitalWrite(PIN_SS, HIGH);

    // NB ! ! ! ! ! ! ! ! ! !
    // Reversed, as FPGA is wired to be a master when talking to the flash device!
    SPIsim mySPI(13, 11, 12);

    // Program FPGA
    digitalWrite(PIN_SS, LOW);                              // Select FPGA, SS = 0
    digitalWrite(PIN_CRESET, LOW);                          // FPGA RESET,  CRESET = 0
    delayMicroseconds(1);                                   // Wait >200ns
    digitalWrite(PIN_CRESET, HIGH);                         // Finish reset CRESET = 1
    delay(1);                                               // Wait >800us

    for (unsigned int i = 0; i < fpgaBinSize; i++) {
        unsigned char b = pgm_read_byte(fpgaBin + i);
        mySPI.transfer(b);
    }
    for (unsigned int i = 0; i < 8; i++) {                  // Required extra bits
        mySPI.transfer(0);                                  // see Lattice FPGA documentation
    }
    return (digitalRead(PIN_CDONE) == HIGH);
}
