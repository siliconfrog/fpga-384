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

#define GATE_A         PIN_A
#define GATE_B         PIN_B
#define TYPE_0         PIN_C
#define TYPE_1         PIN_CE0

#define RESULT         PIN_E

int main(int argc, char *argv[]) {
    if (FPGAprogram((argc < 2) ? "fpga.bin" : argv[1])) {
        std::cout << "Programmed\n";
    } else {
        std::cout << "Failed\n";
    }

    pinMode(GATE_A, OUTPUT);
    pinMode(GATE_B, OUTPUT);
    pinMode(TYPE_0, OUTPUT);
    pinMode(TYPE_1, OUTPUT);
    pinMode(RESULT, INPUT);

    const char *type[] = { " AND ", " OR ", " XOR ", " NOT " };
    for (unsigned char t = 0; t < 4; t++) {
        digitalWrite(TYPE_0, t & 1 ? HIGH : LOW);
        digitalWrite(TYPE_1, t & 2 ? HIGH : LOW);
        for (unsigned char i = 0; i < 4; i++) {
            unsigned char a = i & 1 ? HIGH : LOW;
            unsigned char b = i & 2 ? HIGH : LOW;
            digitalWrite(GATE_A, a);
            digitalWrite(GATE_B, b);

            std::cout << " Input: ";
            if (t == 3) {
                std::cout << type[t];
                std::cout << (int)a;
            } else {
                std::cout << (int)a;
                std::cout << type[t];
                std::cout << (int)b;
            }

            std::cout << "   -> ";
            if (digitalRead(RESULT) == HIGH) {
                std::cout << "HIGH";
            } else {
                std::cout << "LOW";
            }
            std::cout << "\n";
            delay(5000);
        }
    }
}
