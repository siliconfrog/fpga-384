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

#define FLASH_CS                    5

#define SPIFLASH_WRITEENABLE      0x06
#define SPIFLASH_WRITEDISABLE     0x04
#define SPIFLASH_CHIPERASE        0x60        // NB: can take several seconds
#define SPIFLASH_ARRAYREAD        0x0B        // read array of bytes
#define SPIFLASH_BYTEPAGEPROGRAM  0x02        // write (1-256 bytes)
#define SPIFLASH_IDREAD           0x9F

#define SPIFLASH_READ_SIZE        256 * 4 * 8 // 8k for all bytes

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

void (*resetFunc)(void) = 0;                       // reset function @ address 0

// ======================================================
//
// SPI functions to program, read and control the FLASH
//
// ======================================================
void SPIstart(void) {
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV128);
}

void SPIstop(void) {
    SPI.end();
}

void SPIcommand(const uint8_t cmd) {
    digitalWrite(FLASH_CS, LOW);
    delayMicroseconds(1);
    SPI.transfer(cmd);
}

void SPIcommandEnd(void) {
    digitalWrite(FLASH_CS, HIGH);
    delayMicroseconds(10);
}

// ======================================================
//
// Convert two ASCII hex digits to binary
//
// ======================================================
static char hexChar[2];
static uint8_t hexPos = 0;

int readHexChar(unsigned char *b) {
    hexChar[hexPos++] = Serial.read();
    if (hexPos > 1) {
        hexPos = 0;
        unsigned char byteVal = 0;
        if (hexChar[0] >= '0' && hexChar[0] <= '9') {
            byteVal |= (hexChar[0] - '0') << 4;
        }
        if (hexChar[0] >= 'A' && hexChar[0] <= 'F') {
            byteVal |= (hexChar[0] - 'A' + 10) << 4;
        }
        if (hexChar[0] >= 'a' && hexChar[0] <= 'f') {
            byteVal |= (hexChar[0] - 'a' + 10) << 4;
        }
        if (hexChar[1] >= '0' && hexChar[1] <= '9') {
            byteVal |= (hexChar[1] - '0');
        }
        if (hexChar[1] >= 'A' && hexChar[1] <= 'F') {
            byteVal |= (hexChar[1] - 'A' + 10);
        }
        if (hexChar[1] >= 'a' && hexChar[1] <= 'f') {
            byteVal |= (hexChar[1] - 'a' + 10);
        }
        *b = byteVal;
        return 1;
    }
    return 0;
}

void SerialPrintHex(uint8_t h) {
    if (h < 16) {
        Serial.print('0');
    }
    Serial.print(h, HEX);
}

// ======================================================
//
// Command functions
//
// ======================================================
void flashID(void) {
    Serial.print("DeviceID: ");
    SPIcommand(SPIFLASH_IDREAD);                        // Read ID
        uint8_t manufactureID = SPI.transfer(0);
        uint8_t deviceIDa = SPI.transfer(0);
        uint8_t deviceIDb = SPI.transfer(0);
        SerialPrintHex(manufactureID);
        SerialPrintHex(deviceIDa);
        SerialPrintHex(deviceIDb);
        Serial.println("");
    SPIcommandEnd();
    Serial.flush();
}

void flashRead(void) {
    SPIcommand(SPIFLASH_ARRAYREAD);                     // ReadBytes
        SPI.transfer(0);                                // 24bit address
        SPI.transfer(0);
        SPI.transfer(0);
        SPI.transfer(0);                        // dummy
        for (uint16_t i = 0; i < SPIFLASH_READ_SIZE; i++) {
            uint8_t data = SPI.transfer(0);
            if (data < 16) {
                Serial.print("0");
            }
            Serial.print(data, HEX);
        }
    SPIcommandEnd();
}

void flashErase(void) {
    SPIcommand(SPIFLASH_WRITEENABLE);                   // FLASH Write enable
    SPIcommandEnd();

    SPIcommand(SPIFLASH_CHIPERASE);                     // FLASH erase chip
    SPIcommandEnd();
}

void flashWrite(void) {
    bool done = false;

    uint8_t uploadState = 0;
    uint16_t maxCount = 0;
    uint16_t count = 0;

    while (uploadState < 5) {
        switch (uploadState) {
        case 0:
            if (Serial.available() > 0) {
                uint8_t byteVal = Serial.read();
                if (byteVal == 'X') {
                    Serial.write('@');                  // Send sync byte
                    Serial.flush();
                    maxCount = 0;
                    done = false;
                    uploadState = 2;
                }
            }
            break;

        case 2:
            while (!done) {
                if (Serial.available() > 0) {
                    uint8_t byteVal = Serial.read();        // Receive decimal number of bytes
                    if (byteVal >= '0' && byteVal <= '9') {
                        maxCount = maxCount * 10 + (byteVal - '0');
                    } else {
                        done = true;
                        count = 0;
                        uploadState = 3;
                        SPIcommand(SPIFLASH_WRITEENABLE);           // FLASH Write enable
                        SPIcommandEnd();

                        SPIcommand(SPIFLASH_BYTEPAGEPROGRAM);       // FLASH WriteBytes
                        SPI.transfer(0);                            // 24bit address
                        SPI.transfer(count >> 8);
                        SPI.transfer(count & 0xFF);
                    }
                } else {
                    delayMicroseconds(50);
                }
            }
            break;

        case 3:
            if (count < maxCount) {
                if (Serial.available() > 0) {
                    unsigned char byteVal = 0;
                    if (readHexChar(&byteVal) > 0) {
                        if (byteVal < 16) {
                            Serial.print("0");
                        }
                        Serial.print(byteVal, HEX);
                        SPI.transfer(byteVal);
                        ++count;
                        if ((count & 0xFF) == 0) {
                            SPIcommandEnd();
                            if (count > 0) {
                                delay(100);                         // Wait for page to complete
                            }
                            SPIcommand(SPIFLASH_WRITEENABLE);       // FLASH Write enable
                            SPIcommandEnd();

                            SPIcommand(SPIFLASH_BYTEPAGEPROGRAM);   // FLASH WriteBytes
                            SPI.transfer(0);                        // 24bit address
                            SPI.transfer(count >> 8);
                            SPI.transfer(count & 0xFF);
                        }
                    }
                } else {
                    delayMicroseconds(50);
                }
            } else {
                uploadState = 4;
            }
            break;

        case 4:
            SPIcommandEnd();
            delay(50);                                      // Wait for write to complete
            SPIcommand(SPIFLASH_WRITEDISABLE);              // FLASH Write disable
            SPIcommandEnd();

            Serial.write('~');                              // Send response confirmation
            Serial.flush();
            uploadState = 5;
            break;

        default:
            break;
        }
    }
}

void fpgaProgram(void) {
    uint8_t uploadState = 0;
    uint16_t maxCount = 0;
    uint16_t count = 0;
    bool done = false;

    SPIstop();
    pinMode(PIN_CDONE, INPUT);
    pinMode(PIN_CRESET, OUTPUT);
    pinMode(PIN_SS, OUTPUT);
    digitalWrite(PIN_CRESET, HIGH);                         // Not in reset
    digitalWrite(PIN_SS, HIGH);                             // but FPGA not selected

    SPIsim mySPI(13, 11, 12);                               // See FPGA_library.cpp FPGA_program()

    while (uploadState < 5) {
        switch (uploadState) {
        case 0:
            if (Serial.available() > 0) {
                uint8_t byteVal = Serial.read();
                if (byteVal == 'X') {
                    Serial.write('@');                      // Send sync byte
                    Serial.flush();
                    maxCount = 0;
                    done = false;
                    uploadState = 2;
                }
            } else {
                delay(1);
            }
            break;
        case 2:                                             // Program FPGA
            delay(10);
            digitalWrite(PIN_SS, LOW);                      // SS = 0
            digitalWrite(PIN_CRESET, LOW);                  // CRESET = 0
            delayMicroseconds(1);                           // Wait >200ns
            digitalWrite(PIN_CRESET, HIGH);                 // CRESET = 1
            delay(1);                                       // Wait >800us

            while (!done) {
                if (Serial.available() > 0) {
                    uint8_t byteVal = Serial.read();    // Receive decimal number of bytes
                    if (byteVal >= '0' && byteVal <= '9') {
                        maxCount = maxCount * 10 + (byteVal - '0');
                    } else {
                        done = true;
                        count = 0;
                        uploadState = 3;
                    }
                } else {
                    delay(1);
                }
            }
            break;
        case 3:
            if (count < maxCount) {
                if (Serial.available() > 0) {
                    unsigned char byteVal = 0;
                    if (readHexChar(&byteVal) > 0) {
                        if (byteVal < 16) {
                            Serial.print("0");
                        }
                        Serial.print(byteVal, HEX);
                        mySPI.transfer(byteVal);
                        ++count;
                    }
                } else {
                    delayMicroseconds(50);
                }
            } else {
                for (unsigned int i = 0; i < 8; i++) {      // Required extra bits
                    mySPI.transfer(0);
                }
                uploadState = 4;
            }
            break;
        case 4:
            if (digitalRead(PIN_CDONE) == HIGH) {
                Serial.write('~');                          // Send response confirmation
            } else {
                Serial.write('!');                          // Send response failure
            }
            Serial.flush();
            uploadState = 5;
            break;
        default:
            break;
        }
    }
    SPIstart();
}

void setPinsNormal(void) {
    pinMode(PIN_CRESET, OUTPUT);
    digitalWrite(PIN_CRESET, LOW);                          // Hold FPGA in reset
    pinMode(PIN_SS,     OUTPUT);
    pinMode(PIN_CDONE,  INPUT);
    pinMode(FLASH_CS,   OUTPUT);
}

void setInputsIn(void) {
    const uint8_t pins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                            A0, A1, A2, A3, A4, A5, A6, A7 };

    for (uint8_t i = 0; i < sizeof(pins); i++) {
        pinMode(pins[i], INPUT);    // Set ALL arduino pins as inputs, except serial
    }
}

void setup() {
    Serial.begin(115200);

    setInputsIn();

    bool done = false;
    while (!done) {
        if (Serial.available() > 0) {
            if (Serial.read() == 'g') {
                done = true;
            } else {
                delay(1);
            }
        }
    }
    setPinsNormal();
    SPIstart();

    pinMode(FLASH_CS, OUTPUT);
    digitalWrite(FLASH_CS, HIGH);
}

void loop() {
    if (Serial.available() > 0) {
        uint8_t cmd = Serial.read();

        switch(cmd) {
            case 'w':   flashWrite();
                        break;
            case 'r':   flashRead();
                        break;
            case 'i':   flashID();
                        break;
            case 'e':   flashErase();
                        break;
            case 'f':   fpgaProgram();
                        break;
            case '!':   resetFunc();        // Reset
                        break;
            case '<':   setInputsIn();      // Set all as inputs
                        break;
            case '>':   setPinsNormal();    // Set all inputs as normal
                        break;
            default:    break;
        }
    } else {
        delay(1);
    }
}
