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

#define B0              PIN_A
#define B1              PIN_B
#define B2              PIN_C
#define B3              PIN_CE0
#define PARITY          PIN_E

int main(int argc, char *argv[]) {
    if (FPGAprogram((argc < 2) ? "fpga.bin" : argv[1])) {
        std::cout << "Programmed\n";
    } else {
        std::cout << "Failed\n";
    }

    for (int i = 0; i < 10; i++) {
        int counter[4] = { 0, 0, 0, 0 };
        int parity;

        if (digitalRead(B0) == HIGH) {
            counter[0] = 1;
        }
        if (digitalRead(B1) == HIGH) {
            counter[1] = 1;
        }
        if (digitalRead(B2) == HIGH) {
            counter[2] = 1;
        }
        if (digitalRead(B3) == HIGH) {
            counter[3] = 1;
        }

        if (digitalRead(PARITY) == HIGH) {
            parity = 1;
        } else {
            parity = 0;
        }

        std::cout << counter[0] << counter[1] << counter[2] << counter[3];
        std::cout << "    " << parity << "\n";
        delay(5000);
    }
}
