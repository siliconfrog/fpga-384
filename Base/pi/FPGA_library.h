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

#ifndef FPGA_LIBRARY
#define FPGA_LIBRARY

#include <wiringPi.h>

/* From 'gpio readall'
    +-----+-----+---------+------+---+---Pi 2---+---+------+---------+-----+-----+
    | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
    +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
    |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
    |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
    |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
    |   4 |   7 | GPIO. 7 |  OUT | 0 |  7 || 8  | 1 | IN   | TxD     | 15  | 14  |
    |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
    |  17 |   0 | GPIO. 0 |  OUT | 1 | 11 || 12 | 1 | IN   | GPIO. 1 | 1   | 18  |
    |  27 |   2 | GPIO. 2 |  OUT | 0 | 13 || 14 |   |      | 0v      |     |     |
    |  22 |   3 | GPIO. 3 |   IN | 1 | 15 || 16 | 1 | IN   | GPIO. 4 | 4   | 23  |
    |     |     |    3.3v |      |   | 17 || 18 | 1 | IN   | GPIO. 5 | 5   | 24  |
    |  10 |  12 |    MOSI |   IN | 1 | 19 || 20 |   |      | 0v      |     |     |
    |   9 |  13 |    MISO |   IN | 1 | 21 || 22 | 1 | IN   | GPIO. 6 | 6   | 25  |
    |  11 |  14 |    SCLK |   IN | 1 | 23 || 24 | 1 | IN   | CE0     | 10  | 8   |
    |     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
    +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
    | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
    +-----+-----+---------+------+---+---Pi 2---+---+------+---------+-----+-----+
*/

#define PIN_SDA         8
#define PIN_SCL         9
#define PIN_GPIO7       7
#define PIN_GPIO0       0
#define PIN_GPIO2       2
#define PIN_GPIO3       3
#define PIN_MOSI       12
#define PIN_MISO       13
#define PIN_SCLK       14

#define PIN_TXD        15
#define PIN_RXD        16
#define PIN_GPIO1       1
#define PIN_GPIO4       4
#define PIN_GPIO5       5
#define PIN_GPIO6       6
#define PIN_CE0        10
#define PIN_CE1        11

#define PIN_A          PIN_SCLK
#define PIN_B          PIN_MISO
#define PIN_C          PIN_MOSI
#define PIN_D          PIN_GPIO3
#define PIN_E          PIN_GPIO2
#define PIN_F          PIN_GPIO0
#define PIN_G          PIN_GPIO7
#define PIN_H          PIN_SCL
#define PIN_J          PIN_SDA

#define PIN_CRESET     PIN_GPIO3
#define PIN_CDONE      PIN_GPIO5
#define FPGA_CLOCK     PIN_GPIO6    // Must be an INPUT (16MHz oscillator routed here)

extern bool FPGAprogram(const unsigned char *buffer, const unsigned int bufferLen);
extern bool FPGAprogram(const char *fileName);
#endif
