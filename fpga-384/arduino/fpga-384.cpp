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

#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <getopt.h>

using namespace std;

static int fd = 0;                                  // File descriptor for serial port
static struct termios TermOpt;
static bool serialOK = true;

#define SPIFLASH_READ_SIZE      256 * 4 * 8         // 8k for all bytes

// ==================================================================================
//
// readport
//
// Read a character from the serial port
//
// ==================================================================================
int readport(void) {
    char buff;
    int n = read(fd, &buff, 1);
    if (n >= 0) {
        return (int)buff;
    } else if (n == 0) {                            // EOF
        serialOK = false;
    }
    return -1;
}

// ==================================================================================
//
// sendport
//
// Write a character to the serial port
//
// ==================================================================================
int sendport(unsigned char ValueToSend) {
    int n = write(fd, &ValueToSend, 1);

    if (n < 1) {
        serialOK = false;
        return 0;
    }
    return n;
}

// ==================================================================================
//
// openport
//
// Open the specified serial port, and configure 8N1 at specified baud
//
// ==================================================================================
int openport(const char *serialPort, const speed_t baudrate) {
    fd = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)  {
        return -1;
    }

    if (tcgetattr(fd, &TermOpt) < 0) {
        cerr << "ERROR: Couldn't get term attributes\n";
    }
    cfsetispeed(&TermOpt, baudrate);
    cfsetospeed(&TermOpt, baudrate);

    TermOpt.c_cflag &= ~PARENB;
    TermOpt.c_cflag &= ~CSTOPB;
    TermOpt.c_cflag &= ~CSIZE;
    TermOpt.c_cflag |= CS8;
    TermOpt.c_cflag &= ~CRTSCTS;                // no flow control

    TermOpt.c_cflag |= CREAD | CLOCAL;          // turn on READ & ignore ctrl lines
    TermOpt.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    TermOpt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    TermOpt.c_oflag &= ~OPOST;                  // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    TermOpt.c_cc[VMIN]  = 0;
    TermOpt.c_cc[VTIME] = 20;

    if (tcsetattr(fd, TCSANOW, &TermOpt) < 0) {
        cerr << "ERROR: Couldn't set term attributes\n";
        return -2;
    }
    return 0;
}

// ==================================================================================
//
// readbyte
//
// Return a byte read from the serial port, or -1 if nothing recieved
//
// ==================================================================================
int readbyte(void) {
    int rx = -1;
    for (int count = 0; count < 500 && rx < 0; count++) {
        rx = readport();
        if (rx < 0) {
            usleep(1000);
        }
    }
    return rx;
}

static char hexBuffer[] = { 0, 0, 0 };
static int hexBufferPos = 0;

// ==================================================================================
//
// convertHexToBinary
//
// Convert two hexadecimal ASCII characters (0-9A-Fa-f) to byte
//
// ==================================================================================
unsigned char convertHexToBinary(char hi, char lo) {
    unsigned char rxChar = 0;
    if (hi >= '0' && hi <= '9') {
        rxChar |= (hi - '0') << 4;
    }
    if (hi >= 'A' && hi <= 'F') {
        rxChar |= (hi - 'A' + 10) << 4;
    }
    if (lo >= 'a' && hi <= 'f') {
        rxChar |= (hi - 'a' + 10) << 4;
    }
    if (lo >= '0' && lo <= '9') {
        rxChar |= (lo - '0');
    }
    if (lo >= 'A' && lo <= 'F') {
        rxChar |= (lo - 'A' + 10);
    }
    if (lo >= 'a' && lo <= 'f') {
        rxChar |= (lo - 'a' + 10);
    }
    return rxChar;
}

// ==================================================================================
//
// receiveHexChar
//
// Receive a character from the serial port. After a pair received, return a byte
//
// ==================================================================================
int receiveHexChar(unsigned char *b) {
    int rx = readport();
    if (rx >= 0) {
        hexBuffer[hexBufferPos] = rx;
        hexBufferPos++;
        if (hexBufferPos > 1) {
            hexBufferPos = 0;
            *b = convertHexToBinary(hexBuffer[0], hexBuffer[1]);
            return 1;
        }
    }
    return 0;
}

//######################################################################
// Serial protocol byte definitions
#define SYNC_BYTE   '@'
#define DONE_BYTE   '~'
#define XXXX_BYTE   '!'

// ==================================================================================
//
// serialSend
//
// Send a block of data of required length to the serial port.
//
// ==================================================================================
int serialSend(const unsigned char *buffer, const streampos bufLen) {
    sendport('X');                                  // Byte transfer command
    usleep(5000);

    int rx = readbyte();
    if (rx != SYNC_BYTE) {
        cout << "ERROR: Sync got " << (int)rx << endl;
        return -1;
    }

    char bufLenStr[16];
    sprintf(bufLenStr, "%d.", (int)bufLen);         // Send decimal number of data bytes

    for (unsigned int i = 0; i < strlen(bufLenStr); i++) {
        sendport(bufLenStr[i]);
        usleep(20000);
    }
    unsigned char *rxBuffer = new unsigned char[bufLen];
    int rxBufferPtr = 0;
    unsigned char b;
    for (unsigned short int i = 0; i < bufLen; i++) {
        char toHex[] = { 0, 0, 0 };
        sprintf(toHex, "%02X", buffer[i]);
        sendport(toHex[0]);                         // Send ASCII hex byte
        usleep(1000);
        sendport(toHex[1]);
        usleep(1000);

        while ((rxBufferPtr + 16) < i) {            // Receive programmed byte
            if (receiveHexChar(&b) > 0) {           // and keep reading until the lag
                rxBuffer[rxBufferPtr++] = b;        // is at most 16 chars
            }
        }
        if ((i & 0xFF) == 0xFF) {
            cout << "." << flush;
        }
    }

    while (rxBufferPtr < bufLen) {                  // Receive remaining characters
        if (receiveHexChar(&b) > 0) {
            rxBuffer[rxBufferPtr++] = b;
        } else {
            usleep(1000);
        }
    }
    cout << "\nVerifying program data\n";
    bool isOK = true;
    for (unsigned short int i = 0; i < bufLen; i++) {
        if (buffer[i] != rxBuffer[i]) {
            cout << "Error comparing #" << i << " " << (int)buffer[i];
            cout << ":" << (int)rxBuffer[i] << "\n";
            isOK = false;
            break;
        }
    }

    if (isOK) {
        usleep(2500000);                            // End of flashing
        rx = readbyte();
    }
    if (rx == XXXX_BYTE) {
        cout << "ERROR: Failed to program FPGA\n";
        return -2;
    } else if (rx != DONE_BYTE) {
        cout << "ERROR: Done Got " << (int)rx << endl;
        return -3;
    }
    return 0;
}

// ==================================================================================
//
// readFlash
//
// Read the contents of the flash, and output in hexadecimal
//
// ==================================================================================
int readFlash(void) {
    int isErr = 0;
    sendport('r');                                  // Read flash command
    usleep(10000);

    for (int i = 0; i < SPIFLASH_READ_SIZE && !isErr; i++) {
        int hi = readbyte();
        int lo = readbyte();

        if ((i & 15) == 0) {
            cout << endl;
            cout << setfill('0') << setw(4) << hex << i << ": ";
        }

        unsigned char b = convertHexToBinary(hi, lo);
        if (hi < 0 || lo < 0) {
            cout << "\nError reading flash #" <<i << ":" << hi << "," << lo << "\n";
            isErr = 1;
        } else {
            cout << setfill('0') << setw(2) << hex << (int)b << ",";
        }
    }
    cout << endl;
    return isErr;
}

// ==================================================================================
//
// Print help
//
// ==================================================================================
void help(void) {
    cerr << "Usage: fpga-384 [--baud N] [--port PORTNAME] [-a] <file>\n";
    cerr << "   Erase/write <file> to flash, or the FPGA\n\n";
    cerr << "   Defaults: baud 115200, port '/dev/ttyUSB0', <file> 'fpga.bin'\n";
    cerr << "   If -a option, then write flash, read back contents and program FPGA\n";
}

// ==================================================================================
//
// cmdID
//
// get the Flash ID
//
// ==================================================================================
void cmdID(void) {
    cout << "\nGetting ID.....";
    sendport('i');              // Device ID
    usleep(100000);

    int rx;
    do {
        rx = readbyte();
        if (rx > 0) {
            cout << (char)rx;
        }
    } while (rx > 0);
}

// ==================================================================================
//
// cmdEraseFlash
//
// Erase the flash to 0xFF
//
// ==================================================================================
void cmdEraseFlash(void) {
    cout << "Erasing flash...please wait\n";
    sendport('e');              // Flash erase
    usleep(13000000);           // Wait 10s +3s for chip erase
}

// ==================================================================================
//
// cmdReadFlash
//
// Dump contents of flash to output
//
// ==================================================================================
void cmdReadFlash(void) {
   cout << "Reading flash...\n";
    if (readFlash() == 0) {
        usleep(1000000);        // Wait 1s
    }
}

// ==================================================================================
//
// cmdWriteFlash
//
// Write file to flash
//
// ==================================================================================
void cmdWriteFlash(const unsigned char *buffer, const streampos fileLen) {
    cout << "Erase and Write flash ";
    cout << "[0x"<<hex << fileLen << " bytes] " << flush;
    sendport('e');              // Flash erase
    usleep(13000000);           // Wait 10s +3s for chip erase
    sendport('w');              // Write flash
    usleep(100000);             // Wait 100ms
    int ret = serialSend(buffer, fileLen);
    cout << "FLASH programming ";
    if (ret == 0) {
        cout << "Success\n";
    } else {
        cout << "failed, error code " << dec << ret << endl;
    }
}

// ==================================================================================
//
// cmdFPGA
//
// Program the FPGA with file contents
//
// ==================================================================================
void cmdFPGA(const unsigned char *buffer, const streampos fileLen) {
    cout << "Program FPGA [0x"<<hex << fileLen << " bytes] " << flush;
    sendport('f');              // Program FPGA
    usleep(100000);             // Wait 100ms
    int ret = serialSend(buffer, fileLen);
    if (ret == 0) {
        sendport('<');          // All as inputs
        usleep(100000);         // Wait 100ms
    }
    cout << "FPGA programming ";
    if (ret == 0) {
        cout << "Success - Setting all as inputs\n";
    } else {
        cout << "failed, error code " << dec << ret << endl;
    }
}


// ==================================================================================
//
// main
//
// ==================================================================================
int main(int argc, char **argv) {
    speed_t baudrate = B115200;                     // Default baud rate
    const char *serialPort = "/dev/ttyUSB0";        // Default serial port
    const char *fileName = "fpga.bin";              // Default file to program
    bool isAuto = false;
    int c;

    while (1) {
        const struct option long_options[] = {
            {"port",    required_argument, 0, 'p'},
            {"baud",    required_argument, 0, 'b'},
            {"auto",    required_argument, 0, 'a'},
            {"help",   no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        int option_index = 0;     /* getopt_long stores the option index here. */
        c = getopt_long(argc, argv, "p:b:ah", long_options, &option_index);

        if (c == -1) {                              // End of options
            break;
        }

        switch (c) {
        case 'b':
            switch(atoi(optarg)) {
            case 115200:    baudrate = B115200;
                            break;
            case 230400:    baudrate = B230400;
                            break;
            case  57600:    baudrate = B57600;
                            break;
            default:        cerr << "Error: Unknown baudrate, use 57600/115200/230400 only\n";
                            exit(1);
            }
            break;
        case 'p':           serialPort = optarg;
                            break;
        case 'a':           isAuto = true;
                            break;
        case 'h':           help();
                            exit(0);
        default:            exit(5);                // Unknown option
                            break;
        }
    }

    if (optind < argc) {
        fileName = argv[optind++];
    }

    int ret = 0;
    unsigned char *buffer = nullptr;
    streampos fileLen = 0;
    int rx;

    ifstream f;
    f.open(fileName, ios::in | ios::binary | ios::ate);
    if (f.is_open()) {
        fileLen = f.tellg();
        f.seekg(0, ios::beg);
        buffer = new unsigned char[fileLen];
        f.read(reinterpret_cast<char *>(buffer), fileLen);
        f.close();
        if (buffer == nullptr || fileLen == 0) {
            cerr << "ERROR: No data to send\n";
            ret = -4;
        }
    } else {
        help();
        cerr << "ERROR: Can't open '" << fileName << "'\n";
        ret = -5;
    }
    if (ret == 0) {
        if (openport(serialPort, baudrate) == 0) {
            usleep(2000000);                        // Wait for serial to settle
            sendport('g');                          // GO (out of halt state)
            usleep(500000);                         // Wait 500ms to reset FPGA and prepare

            bool quit = false;
            while (!quit) {
                do {
                    rx = readbyte();                // Clear serial buffer if needed
                    usleep(10000);
                } while (rx > 0);

                cout << "\nCommands:\n";
                cout << "\ti\tGet ID\n\tr\tRead Flash\n\te\tErase Flash\n";
                cout << "\tw\tWrite (and Erase) Flash\n\tf\tProgram FPGA\n";
                cout << "\t>\tI/P normal\n\t<\tAll Inputs\n\t!\tReset\n";
                cout << "\ta\tAll - Get ID, Erase/Write/Read Flash, program FPGA and exit\n";
                cout << "\tx\tExit\n\n";

                int cmd = 0;
                if (!isAuto) {
                    do {
                        cmd = getchar();
                    } while (cmd == '\n' || cmd == '\r');
                } else {
                    cmd = 'a';
                }

                switch (cmd) {
                    case 'i':   cmdID();
                                break;
                    case 'r':   cmdReadFlash();
                                break;
                    case 'e':   cmdEraseFlash();
                                break;
                    case 'w':   cmdWriteFlash(buffer, fileLen);
                                break;
                    case 'f':   cmdFPGA(buffer, fileLen);
                                break;
                    case '<':   sendport('<');              // Connections as input command
                                usleep(100000);             // Wait 100ms
                                break;
                    case '>':   sendport('>');              // Normal connections command
                                usleep(100000);             // Wait 100ms
                                break;
                    case '!':   sendport('!');              // Program FPGA command
                                usleep(100000);             // Wait 100ms
                                sendport('g');              // GO (out of halt state)
                                usleep(500000);             // Wait 500ms to reset FPGA and prepare
                                break;
                    case 'x':   quit = true;
                                break;
                    case 'a':   cmdID();
                                cmdWriteFlash(buffer, fileLen);
                                cmdReadFlash();
                                cmdFPGA(buffer, fileLen);
                                quit = true;
                                break;
                    default:    cout << "Unknown command: " << (int)cmd << endl;
                                break;
                }
                if (!serialOK) {
                    quit = true;
                }
            }
            delete [] buffer;
        } else {
            cerr << "ERROR: Unable to open port '" << serialPort << "'\n";
        }
    }
    return ret;
}
