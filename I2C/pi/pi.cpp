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
#include "FPGA_library.h"
#include <wiringPiI2C.h>

#define I2C_ADDRESS     0x24

int main(int argc, char *argv[]) {
    if (FPGAprogram((argc < 2) ? "fpga.bin" : argv[1])) {
        std::cout << "Programmed\n";
    } else {
        std::cout << "Failed\n";
        return 1;
    }

    /* Reset the I2C pins in case they have been used previously. */
    pinModeAlt(PIN_SDA, 0b100);
    pinModeAlt(PIN_SCL, 0b100);
    int fd = wiringPiI2CSetup(I2C_ADDRESS);

    if (fd < 0) {
        std::cerr << "Error setting up I2C\n";
    } else {
        for (int counter = 0; counter < 16; counter++) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << 
                                counter << " " << std::flush;
            wiringPiI2CWrite(fd, counter);
            delay(200);
        }
        std::cout << "\n";
    }
        
    return 0;
}
