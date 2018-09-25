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
#include <stdlib.h>
#include "FPGA_library.h"

#define ADDER_A         PIN_E
#define ADDER_B         PIN_F
#define ADDER_CIN       PIN_G

#define SUM_IN          PIN_H
#define COUT_IN         PIN_J

int main(int argc, char *argv[]) {
    if (FPGAprogram((argc < 2) ? "fpga.bin" : argv[1])) {
        std::cout << "Programmed\n";
    } else {
        std::cout << "Failed\n";
	return 1;
    }

    pinMode(ADDER_A, OUTPUT);
    pinMode(ADDER_B, OUTPUT);
    pinMode(ADDER_CIN, OUTPUT);
    pinMode(SUM_IN, INPUT);
    pinMode(COUT_IN, INPUT);

    for (int ctr = 0; ctr < 16; ctr++) {
        if ((ctr & 1) > 0) {
            std::cout << "A high ";
            digitalWrite(ADDER_A, HIGH);
        } else {
            std::cout << "A low  ";
            digitalWrite(ADDER_A, LOW);
        }
        if ((ctr & 2) > 0) {
            std::cout << "B high ";
            digitalWrite(ADDER_B, HIGH);
        } else {
            std::cout << "B low  ";
            digitalWrite(ADDER_B, LOW);
        }
        if ((ctr & 4) > 0) {
            std::cout << "Cin high ";
            digitalWrite(ADDER_CIN, HIGH);
        } else {
            std::cout << "Cin low  ";
            digitalWrite(ADDER_CIN, LOW);
        }

        std::cout << "         ";

        if (digitalRead(SUM_IN) == HIGH) {
            std::cout << "SUM high  ";
        } else {
            std::cout << "SUM low   ";
        }
        if (digitalRead(COUT_IN) == HIGH) {
            std::cout << "COUT high";
        } else {
            std::cout << "COUT low ";
        }
        std::cout << "\n";
        delay(5000);
    }
    return 0;
}
