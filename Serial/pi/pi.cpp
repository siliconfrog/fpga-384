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
#include <stdlib>
#include <unistd.h>
#include <string.h>
#include "FPGA_library.h"
#include <wiringSerial.h>

/* Note that this program must be run as sudo fpga_pi.
 *
 * Also, ensure the serial port is enabled and the system console
 * is disabled, which can be done from raspi-config.
 */

int main(int argc, char *argv[]) {
    if (FPGAprogram((argc < 2) ? "fpga.bin" : argv[1]))) {
        std::cout << "Programmed\n";
    } else {
        std::cout << "Failed\n";
    }

    int fd = serialOpen("/dev/ttyAMA0", 9600);
    if (fd < 0) {
        std::cerr << "Error opening /dev/ttyAMA0 at 9600 baud\n";
        exit(1);
    }
    delay(100);
    serialFlush(fd);

    int res = 0;
    for (int chTx = 'a'; chTx <= 'z' && res >= 0; chTx++) {
        serialPutchar(fd, (unsigned char)chTx);
        bool success = false;
        int chRx = 0;

        for (int tries = 0; tries < 5 && !success; tries++) {
            delay(10);
            res = serialDataAvail(fd);
            if (res > 0) {
                chRx = serialGetchar(fd);
                if (chRx > 0) {
                    success = true;
                    std::cout << "Tx '" << (char)chTx;
                    std::cout << "'   Rx '" << (char)chRx << "'\n";
                }
            }
        } 
        if (res < 0) {
            std::cout << "Error from serialDataAvail: " << strerror(errno) << "\n";
        } else if (chRx < 0) {
            res = chRx;
            std::cout << "Error from serialGetChar: " << strerror(errno) << "\n";
        }
        serialFlush(fd);
        delay(100);
    }
    serialClose(fd);
}
