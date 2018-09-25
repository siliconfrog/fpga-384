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

#include <string.h>
#include "FPGA_library.h"
#include "fpgabin.h"

#define LED             13
#define DATA_PIN         6
#define CLK             13          // SCK
#define ENABLE          10          // SS - active LOW

#define ENABLE_ACTIVE   LOW
#define ENABLE_INACTIVE HIGH

/* Connections:  D6 -> FPGA "E" */


void setup(void) {
    bool fProg = FPGAprogram(fpgaBin, sizeof(fpgaBin));
    Serial.begin(115200);
    if (fProg) {
        Serial.println("Programmed");
    } else {
        Serial.println("Failed");           // Failed - flash arduino LED
        while(1) {
            digitalWrite(LED, LOW);
            delay(50);
            digitalWrite(LED, HIGH);
            delay(50);
        }
    }
    pinMode(DATA_PIN, OUTPUT);      digitalWrite(DATA_PIN, LOW);
    pinMode(CLK, OUTPUT);       digitalWrite(CLK, LOW);
    pinMode(ENABLE, OUTPUT);    digitalWrite(ENABLE, ENABLE_INACTIVE);
}

const char *pattern = "0101011101001010101101011101111011101101010100010111100110000100";
bool patternSent = false;

void loop(void) {
    if (!patternSent) {
        digitalWrite(ENABLE, ENABLE_ACTIVE);
        for (int i = strlen(pattern) - 1; i >= 0; i--) {
            bool bit = (pattern[i] == '1');
            digitalWrite(DATA_PIN, bit ? HIGH : LOW);

            delayMicroseconds(1);
            digitalWrite(CLK, HIGH);    // Pulse the clock
            delayMicroseconds(1);
            digitalWrite(CLK, LOW);
            delay(150);
        }
        patternSent = true;
        digitalWrite(ENABLE, ENABLE_INACTIVE);
    } else {
        digitalWrite(LED, LOW);
        delay(250);
        digitalWrite(LED, HIGH);
        delay(250);
    }
}
