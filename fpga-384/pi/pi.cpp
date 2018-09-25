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
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "FPGA_library.h"

using namespace std;

#define SPIFLASH_WRITEENABLE      0x06
#define SPIFLASH_WRITEDISABLE     0x04
#define SPIFLASH_CHIPERASE        0x60        // NB: can take several seconds
#define SPIFLASH_ARRAYREAD        0x0B        // read array of bytes
#define SPIFLASH_BYTEPAGEPROGRAM  0x02        // write (1-256 bytes)
#define SPIFLASH_IDREAD           0x9F

#define SPIFLASH_READ_SIZE        256 * 4 * 8 // 8k for all bytes

unsigned char SPItransfer(const unsigned char b) {
    unsigned char res = 0;
    unsigned char mask = 0x80;

    for (int i = 0; i < 8; i++) {
        if (b & mask) {
            digitalWrite(PIN_MOSI, HIGH);
        } else {
            digitalWrite(PIN_MOSI, LOW);
        }
        usleep(1);
        digitalWrite(PIN_SCLK, LOW);             // HIGH -> LOW : Clock in data
        usleep(1);
        digitalWrite(PIN_SCLK, HIGH);            // LOW -> HIGH : Clock out result
        usleep(1);

        if (digitalRead(PIN_MISO) == HIGH) {
            res |= mask;
        }
        mask >>= 1;
    }
    return res;
}

void SPIcommand(const unsigned char cmd) {
    digitalWrite(PIN_CE1, LOW);
    usleep(1);
    SPItransfer(cmd);
}

void SPIcommandEnd(void) {
    digitalWrite(PIN_CE1, HIGH);
    usleep(10);
}

void flashID(void) {
    cout << "DeviceID: ";
    SPIcommand(SPIFLASH_IDREAD);                // Read ID
        cout << setw(2) << setfill('0') << hex << (int)SPItransfer(0);
        cout << setw(2) << setfill('0') << hex << (int)SPItransfer(0);
    SPIcommandEnd();
    cout << "\n";
}

void flashRead(void) {
    cout << "Read from flash\n";
    SPIcommand(SPIFLASH_ARRAYREAD);             // ReadBytes
        SPItransfer(0);                         // 24bit address
        SPItransfer(0);
        SPItransfer(0);
        SPItransfer(0);                         // dummy
        for (unsigned short i = 0; i < SPIFLASH_READ_SIZE; i++) {
            if ((i & 15) == 0) {
                cout << "\n" << setw(4) << setfill('0') << hex << i << ": ";
            }
            unsigned char data = SPItransfer(0);
            cout << setw(2) << setfill('0') << hex << (int)data << " ";
        }
        cout << "\n";
    SPIcommandEnd();
}

void flashErase(void) {
    cout << "Erase flash....please wait\n";
    SPIcommand(SPIFLASH_WRITEENABLE);           // FLASH Write enable
    SPIcommandEnd();

    SPIcommand(SPIFLASH_CHIPERASE);             // FLASH erase chip - done in background
    SPIcommandEnd();
    usleep(1000000 * 10);
}

void flashWrite(const unsigned char *buffer, const unsigned short bufferLen) {
    flashErase();                               // Erase flash first

    cout << "Write flash\n";
    SPIcommand(SPIFLASH_WRITEENABLE);           // FLASH Write enable
    SPIcommandEnd();

    SPIcommand(SPIFLASH_BYTEPAGEPROGRAM);       // FLASH WriteBytes
        SPItransfer(0);                         // 24bit address
        SPItransfer(0);
        SPItransfer(0);

    for (unsigned short count = 0; count < bufferLen;) {
        SPItransfer(buffer[count++]);
        if ((count & 0xFF) == 0) {
            SPIcommandEnd();
            delay(50);                          // Wait for page to complete
            SPIcommand(SPIFLASH_WRITEENABLE);   // FLASH Write enable
            SPIcommandEnd();

            
            SPIcommand(SPIFLASH_BYTEPAGEPROGRAM);// FLASH WriteBytes
                SPItransfer(0);                 // 24bit address
                SPItransfer(count >> 8);
                SPItransfer(count & 0xFF);
        }
    }

    SPIcommandEnd();
    delay(50);                                  // Wait for write to complete
    SPIcommand(SPIFLASH_WRITEDISABLE);          // FLASH Write disable
    SPIcommandEnd();
}

void fpgaReset(void) {
    pinMode(PIN_CRESET, OUTPUT);
    digitalWrite(PIN_CRESET, LOW);              // Hold FPGA in reset

    pinMode(PIN_MISO, INPUT);
    pinMode(PIN_MOSI, OUTPUT);
    digitalWrite(PIN_MOSI, LOW);
    pinMode(PIN_SCLK, OUTPUT);
    digitalWrite(PIN_SCLK, HIGH);
}

void fpgaProgram(const unsigned char *buffer, const unsigned int bufferLen) {
    if (buffer != nullptr) {
        if (FPGAprogram(buffer, bufferLen)) {
            cout << "FPGA program: Success\n";
        } else {
            cout << "FPGA program: Failed\n";
        }
    }
}

static unsigned char *buffer = nullptr;
static unsigned int bufferLen = 0;

bool readBuffer(const char *fileName) {
    bool ok = false;

    ifstream f;
    f.open(fileName, ios::in | ios::binary | ios::ate);
    if (f.is_open()) {
        streampos fileLen = f.tellg();
        f.seekg(0, ios::beg);
        if (fileLen < SPIFLASH_READ_SIZE) {
            bufferLen = 8u + (unsigned int)fileLen;
            buffer = new unsigned char[bufferLen];
            memset(buffer, 0, bufferLen);
            f.read(reinterpret_cast<char *>(buffer), bufferLen);
            ok = true;
        }
        f.close();
    }
    return ok;
}

void help(void) {
    cerr << "Usage:  fpga-384-control  <options> [<file>]\n";
    cerr << "Options:\n";
    cerr << "     [-h|--help]           Show this help\n";
    cerr << "     [-p|--program]        Program the FPGA. Requires <file>\n";
    cerr << "     [-r|--read]           Show the flash contents\n";
    cerr << "     [-w|--write]          Write the data to the flash. Requires <file>\n";
    cerr << "     [-e|--erase]          Erase the flash\n";
    cerr << "     [-d|--id]             Get the flash ID\n";
    cerr << "     [-i|--interactive]    Interactive mode\n";
    cerr << "Program <file> to FPGA\n\n";
}

int main(int argc, char *argv[]) {
    bool doProgram = false;
    bool doRead = false;
    bool doWrite = false;
    bool doErase = false;
    bool doID = false;
    bool isInteractive = false;
    const char *fileName = nullptr;

    while (1) {
        const struct option long_options[] = {
            { "program",     no_argument, 0, 'p' },
            { "read",        no_argument, 0, 'r' },
            { "write",       no_argument, 0, 'w' },
            { "erase",       no_argument, 0, 'e' },
            { "interactive", no_argument, 0, 'i' },
            { "id",          no_argument, 0, 'd' },
            { "help",        no_argument, 0, 'h' },
            { 0, 0, 0, 0 }
        };
        int option_index = 0;
        int c = getopt_long(argc, argv, "prweidh", long_options, &option_index);

        if (c == -1) {          // End of options
            break;
        }

        switch (c) {
        case 'p':   doProgram = true;
                    break;
        case 'r':   doRead = true;
                    break;
        case 'w':   doWrite = true;
                    break;
        case 'e':   doErase = true;
                    break;
        case 'i':   isInteractive = true;
                    break;
        case 'd':   doID = true;
                    break;
        case 'h':   help();
                    exit(0);
        default:    return 1;   // Unknown option
                    break;
        }
    }

    if ((doRead && doWrite) || ((doRead || doWrite) && doErase)) {
        cout << "\nError - invalid read/write flash operation\n\n";
        exit(5);
    }
    if (optind < argc) {
        fileName = argv[optind++];
        if (!readBuffer(fileName)) {
            cerr << "Error reading file '" << fileName << "'\n";
            exit(5);
        }
    } else {
        if (doProgram || doWrite) {
            cerr << "\nError - missing data file\n\n";
            help();
            exit(5);
        }
        if (isInteractive) {
            cout << "\nSpecify fpga.bin file be able to write to flash / program FPGA";
        }
        if (doErase && doWrite) {
            doErase = false;            // Writeflash automatically erases flash
        }

        if (!doProgram && !doRead && !doWrite && !doErase && !isInteractive) {
            help();
            exit(5);
        }
    }

    int wpi_device = wiringPiSetup();
    if (wpi_device == -1) {
        cout << "Wiring Pi failed to set up\n";
        exit(5);
    }

    // Start up making all pins input
    pinMode(PIN_CRESET, INPUT);
    pinMode(PIN_CE0, INPUT);
    pinMode(PIN_CDONE, INPUT);
    pinMode(PIN_CE1, INPUT);
    fpgaReset();

    pinMode(PIN_CE1, OUTPUT);
    digitalWrite(PIN_CE1, HIGH);

    if (!isInteractive) {
        if (doWrite) {
            flashWrite(buffer, bufferLen);
        }
        if (doRead) {
            flashRead();
        }
        if (doID) {
            flashID();
        }
        if (doErase) {
            flashErase();
        }
        if (doProgram) {
            fpgaProgram(buffer, bufferLen);
        }
    } else {
        bool quit = false;

        while (!quit) {
            cout << "\nCommands:\n";
            cout << "\ti\tFlash ID\n\tr\tRead flash\n\te\tErase flash\n";
            if (buffer != nullptr) {
                cout << "\tw\tErase and Write flash\n\tp\tProgram FPGA only\n";
            }
            cout << "\t0\tFPGA reset\n\tq\tQuit\n";
            cout << "Ready: ";
            int cmd = 0;
            do {
                cmd = getchar();
            } while (cmd == '\r' || cmd == '\n');
            cout << "\n";

            switch(cmd) {
                case 'w':   flashWrite(buffer, bufferLen);
                            break;
                case 'r':   flashRead();
                            break;
                case 'i':   flashID();
                            break;
                case 'e':   flashErase();
                            break;
                case 'p':   fpgaProgram(buffer, bufferLen);
                            break;
                case '0':   fpgaReset();
                            break;
                case 'q':   quit = true;
                            break;
                default:    cout << "Unknown command '" << cmd << "'\n";
                            break;
            }
        }
    }
    if (buffer != nullptr) {
        delete [] buffer;
    }

    return 0;
}
