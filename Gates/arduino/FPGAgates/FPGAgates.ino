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

#include "FPGA_library.h"
#include "fpgabin.h"

#define LED             13


#define GATE_A         13   // D13:  Gate input A
#define GATE_B         12   // D12:  Gate input B
#define TYPE_0         11   // D11:  Select gate type A
#define TYPE_1         10   // D10:  Select gate type B

#define RESULT         A0   // '5' == LED_BLUE output

/* Connections (as for programming) are:
 *  13 -> FPGA 'A',    12 -> FPGA 'B',    11 -> FPGA 'C',     10 -> FPGA '12'
 *  also connect A0 -> FPGA '5' to read the result.
 */

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
    pinMode(GATE_A, OUTPUT);
    pinMode(GATE_B, OUTPUT);
    pinMode(TYPE_0, OUTPUT);
    pinMode(TYPE_1, OUTPUT);
    pinMode(RESULT, INPUT);

}

void loop(void) {
    const char *type[] = { " AND ", " OR ", " XOR ", " NOT " };
    for (unsigned char t = 0; t < 4; t++) {
        digitalWrite(TYPE_0, t & 1 ? HIGH : LOW);
        digitalWrite(TYPE_1, t & 2 ? HIGH : LOW);
        for (unsigned char i = 0; i < 4; i++) {
            unsigned char a = i & 1 ? HIGH : LOW;
            unsigned char b = i & 2 ? HIGH : LOW;
            digitalWrite(GATE_A, a);
            digitalWrite(GATE_B, b);

            Serial.print(" Input: ");
            if (t == 3) {
                Serial.print(type[t]);
                Serial.print(a, DEC);
            } else {
                Serial.print(a, DEC);
                Serial.print(type[t]);
                Serial.print(b, DEC);
            }

            Serial.print("   -> ");
            if (digitalRead(RESULT) == HIGH) {
                Serial.print("HIGH");
            } else {
                Serial.print("LOW");
            }
            Serial.println("");

            delay(5000);
        }
    }
}
