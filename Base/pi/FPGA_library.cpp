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
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include "FPGA_library.h"

#include <wiringPi.h>
#include <wiringPiSPI.h>

class Gpio {
    public:
        Gpio() {
            name = std::string("");
            pin = 0;
            direction = INPUT;
            value = LOW;
        }

        void Initialise(const std::string & name_in,
                        const unsigned char pin_in,
                        const int dir_in = INPUT,
                        const int value_in = LOW) {
            name = name_in;
            pin = pin_in;
            value = value_in;

            SetDir(dir_in);
            if (direction == OUTPUT) {
                Set(value);
            }
        }

        ~Gpio() {
            if (direction == OUTPUT) {
                SetDir(INPUT);
            }
        }

        void SetDir(const int newdir) {
            pullUpDnControl(pin, PUD_OFF);
            pinMode(pin, newdir);

            direction = newdir;
        }

        void Set(const int new_value) {
            if (direction == OUTPUT) {
                digitalWrite(pin, new_value);
                value = new_value;
            } else {
                std::cout << "Error assigning value to gpio input Pin: " << name
                         << " to " << new_value << "\n";
                exit(1);
            }
        }

        int Get() {
            if (direction == INPUT) {
                value = digitalRead(pin);
            } else {
                std::cout << "Error getting value from output gpio Pin: " << name << "\n";
                exit(2);
            }
            return value;
        }

    private:
        std::string name;
        unsigned char pin;
        int direction;
        int value;
};

class SPI {
    public:
        SPI() {
            wpi_device = wiringPiSetup();
            if (wpi_device == -1) {
                std::cerr << "WiringPi failed to set up\n";
                exit(1);
            }
        }
        
        ~SPI() {
            if (wpi_device != -1) {
                close(wpi_device);
                wpi_device = -1;
            }
        }

        bool Initialise() {
            bool ok = (wpi_device != -1);

            if (ok) {
                //                           FPGA pin    Raspi
                creset.Initialise("CRESET", PIN_CRESET, OUTPUT, LOW);
                cdone.Initialise("CDONE", PIN_CDONE, INPUT);

                // Program using bit bang so set pins using GPIO
                clk.Initialise("CLK", PIN_SCLK, OUTPUT, LOW);

                // NB ! ! ! ! ! ! ! ! ! !
                // Reversed, as FPGA is wired to be a master when talking to the flash device!
                mosi.Initialise("MOSI", PIN_MISO, OUTPUT, LOW);
                miso.Initialise("MISO", PIN_MOSI, INPUT);

                ce0.Initialise("CE0", PIN_CE0, OUTPUT, HIGH);
                ce1.Initialise("CE1", PIN_CE1, OUTPUT, HIGH);
            }
            return ok;
        }

        void Start() {
            ce0.Set(LOW);
            creset.Set(LOW);
            usleep(1);		    // Wait > 200ns
            creset.Set(HIGH);
            usleep(2000);		// Wait > 800us
        }

        bool SendBytes(const unsigned char *bytes, const unsigned int length) {
            bool ok = true;
            unsigned int buf_ptr = 0;
            const unsigned char *p_block;
            unsigned char bit;

            //std::cout << "Sending " << length << " bytes." << endl;

            // Bit bang them out MSB first with +ve as 1 and GND as 0
            while (ok && buf_ptr < length) {
                p_block = bytes + buf_ptr;
                //cout << "Byte = 0x" << hex << fixed << setw(2) << *p_block << endl;

                bit = 0x80;
                while (bit != 0) {
                    // Forget about the read on the MISO

                    if (*p_block & bit) {
                        mosi.Set(HIGH);
                    } else {
                        mosi.Set(LOW);
                    }
                    for (volatile int n = 0; n < 100; n++) { }
                    clk.Set(HIGH);
                    for (volatile int n = 0; n < 100; n++) { }
                    clk.Set(LOW);
                    bit >>= 1;
                }
                buf_ptr++;
            }
            mosi.Set(LOW);
            for (unsigned int i = 0; i < 8*8; i++) {                  // Required extra bits
                for (volatile int n = 0; n < 100; n++) { }
                clk.Set(HIGH);
                for (volatile int n = 0; n < 100; n++) { }
                clk.Set(LOW);
            }
            ce0.Set(HIGH);
            return ok;
        }

        bool IsProgrammed() {
            return cdone.Get() == HIGH;
        }

    private:
        Gpio creset, cdone;                 // Actual GPIO pins
        Gpio clk, mosi, miso, ce0, ce1;     // SPI pins
        int wpi_device;
};

///////////////////////////////////////////////////////////////////////////////////////////
static SPI spi;

bool FPGAprogram(const unsigned char *buffer, const unsigned int bufferLen) {
    bool ok = false;
    if (spi.Initialise()) {
        spi.Start();		    // CRESET=0, SS=0, dly, CRESET=1
        if (spi.SendBytes(buffer, bufferLen)) {
            usleep(10000);		// Allow time(10ms) for the FPGA to pull up CDONE

            if (spi.IsProgrammed()) {
                ok = true;
                pinMode(PIN_GPIO6,  INPUT); 	// Must be an input (16MHz oscillator)
            }
        }
    }
    return ok;
}

bool FPGAprogram(const char *fileName) {
    bool ok = false;

    std::ifstream f;
    f.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
    if (f.is_open()) {
        std::streampos fileLen = f.tellg();
        f.seekg(0, std::ios::beg);
        unsigned char *buffer = new unsigned char[fileLen];
        f.read(reinterpret_cast<char *>(buffer), fileLen);
        f.close();

        ok = FPGAprogram(buffer, fileLen);
        delete [] buffer;
    }
    return ok;
}
