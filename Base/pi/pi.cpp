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
#include <fstream>
#include <stdlib.h>
#include <getopt.h>
#include "FPGA_library.h"

int main(int argc, char *argv[]) {
    const char *fileName = "fpga.bin";      // default file to program

    while (1) {
        const struct option long_options[] = {
            {"help",   no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c = getopt_long(argc, argv, "h", long_options, &option_index);

        if (c == -1) {          // End of options
            break;
        }

        switch (c) {
        case 'h':   std::cerr << "Usage:  " << argv[0] << " [-h|--help] <file>\n";
                    std::cerr << "    Program <file> to FPGA - default is 'fpga.bin'\n";
                    exit(0);
        default:    return 1;   // Unknown option
                    break;
        }
    }

    if (optind < argc) {
        fileName = argv[optind++];
    }

    std::ifstream f;
    f.open(fileName);
    if (f.is_open()) {
        f.close();
        std::cout << "Programming FPGA with data from '" << fileName << "'\n";
        if (FPGAprogram(fileName)) {
            std::cout << "FPGA Programmed successfully\n";
        } else {
            std::cout << "FPGA Programming Failed\n";
            return 1;
        }
    } else {
        std::cerr << "Error opening FPGA data file '" << fileName << "'\n";
        return 1;
    }
    return 0;
}
