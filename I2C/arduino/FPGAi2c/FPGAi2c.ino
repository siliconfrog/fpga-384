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

// ===========================================================

// Connect A4 -> "J" and A5 -> "H"

// TYPE = 0 for manual
// TYPE = 1 for fastwire
// TYPE = 2 for wire
#define TYPE    2


// ===========================================================

#define I2C_ADDR        0x24
uint8_t counter = 0;

#if TYPE == 0

// Manual I2C bit-banging

#define SDA         A4
#define SCL         A5
#define ADDR(A, B)  ((((A) & (1<<(B))) > 0) ? HIGH : LOW)
#define REG(R, B)   ((((R) & (1<<(B))) > 0) ? HIGH : LOW)
#define DELAY(X)    delay(X)

void I2Cmanual_Setup(void) {
    digitalWrite(SCL, HIGH);
    digitalWrite(SDA, HIGH);

    pinMode(SCL, OUTPUT);
    pinMode(SDA, OUTPUT);
}

void I2Cmanual_write(const uint8_t addr, const uint8_t reg) {
    uint8_t x = 0;

    DELAY(5);   digitalWrite(SDA,  LOW);        digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW); // START

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 6) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 5) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 4) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 3) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 2) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 1) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  ADDR(addr, 0) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  LOW);        DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW); // write bit

    pinMode(SDA, INPUT_PULLUP);
    DELAY(5);                                   digitalWrite(SCL, HIGH);
    DELAY(5);   x = digitalRead(SDA);            digitalWrite(SCL, LOW); // addr ack
    Serial.print("ADDR ACK x=");
    Serial.print(x, DEC);
    Serial.println("\r");
    pinMode(SDA, OUTPUT);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 7) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 6) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 5) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 4) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 3) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 2) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 1) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    DELAY(2);   digitalWrite(SDA,  REG(reg, 0) );   DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                       digitalWrite(SCL, LOW);

    pinMode(SDA, INPUT_PULLUP);
    DELAY(5);                                       digitalWrite(SCL, HIGH);
    DELAY(5);   x = digitalRead(SDA);               digitalWrite(SCL, LOW); // data ack
    Serial.print("ADDR ACK x=");
    Serial.print(x, DEC);
    Serial.println("\r");
    pinMode(SDA, OUTPUT);

    pinMode(SDA, OUTPUT);
    DELAY(2);   digitalWrite(SDA,  HIGH);       DELAY(3);   digitalWrite(SCL, HIGH);
    DELAY(5);                                   digitalWrite(SCL, LOW); // STOP
    DELAY(5);                                   digitalWrite(SCL, HIGH); // STOP
}
#define I2C_SETUP       I2Cmanual_Setup
#define I2C_WRITE       I2Cmanual_write

#elif TYPE == 1         // Fastwire example

#include "I2Cdev.h"
/* static int8_t readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length,
 *                         uint8_t *data, uint16_t timeout=I2Cdev::readTimeout);
 * static bool writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
*/
#define I2C_SPEED       100

void I2Cfastwire_Setup(void) {
    Fastwire::setup(I2C_SPEED, 1);          // speed, bool pullup
}

void I2Cfastwire_write(const uint8_t addr, const uint8_t reg) {
    const uint8_t dataLen = 0;
    uint8_t data[] = { 0 };
    I2Cdev::writeBytes(addr, reg, dataLen, data);
}

#define I2C_SETUP       I2Cfastwire_Setup
#define I2C_WRITE       I2Cfastwire_write

#elif TYPE == 2         // wire example

#include <Wire.h>
void I2Cwire_Setup(void) {
    Wire.begin();
}

void I2Cwire_write(const uint8_t addr, const uint8_t reg) {
    Wire.beginTransmission(addr);           // Send I2C address
    Wire.write(reg);                        // Send register address
                                            // Normally send data: Wire.write(value);
    Wire.endTransmission();
}

#define I2C_SETUP       I2Cwire_Setup
#define I2C_WRITE       I2Cwire_write

#endif

void setup() {
    bool fProg = FPGAprogram(fpgaBin, sizeof(fpgaBin));
    Serial.begin(115200);
    if (fProg) {
        Serial.println("Programmed");
    } else {
        Serial.println("Failed");           // Failed - flash arduino LED quickly
        pinMode(LED, OUTPUT);
        while(1) {
            digitalWrite(LED, LOW);
            delay(50);
            digitalWrite(LED, HIGH);
            delay(50);
        }
    }

    Serial.println("Setup");
    pinMode(PIN_SS, OUTPUT);
    digitalWrite(PIN_SS, HIGH);
    pinMode(LED, OUTPUT);

    I2C_SETUP();
}

void loop() {
    I2C_WRITE(I2C_ADDR, counter);

    Serial.print(counter, HEX);
    Serial.println("");

    delay(125);
    counter++;
}
