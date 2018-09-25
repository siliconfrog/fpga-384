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

// LSFR - Linear Shift Feedback Register: a pseudo-random counter

// See: https://en.wikipedia.org/wiki/Linear-feedback_shift_register

// NB: To see the contents of the whole shift register, enable LFSRout here
//     and in the pin-configuration file (ice40-384_rand.pcf) - the
//     LED_RED pin will also need to be changed to avoid conflicts.
//     The contents of the shift register will be placed on the DIL 1-9 pins

module LFSR8(clk, ledR, ledG, ledB /*, LFSRout*/);
    input clk;
    output ledR, ledG, ledB;
    //output [8:0] LFSRout;

    reg [19:0] osc = 0;
    reg [8:0] LFSR = 9'h1;          // initial value (must not be 0!)

    // A 9-bit LFSR has maximum length with taps at bits 9, 5, 1
    wire feedback = (LFSR[0] ^ LFSR[4]) ^ LFSR[8];

    assign ledR = LFSR[8];
    assign ledG = 0;
    assign ledB = 0;
    //assign LFSRout[8:0] = LFSR[8:0];

    always @(posedge clk) begin
        osc <= osc + 1;
    end

    always @(posedge osc[19]) begin
        LFSR[7] <= LFSR[8];
        LFSR[6] <= LFSR[7];
        LFSR[5] <= LFSR[6];
        LFSR[4] <= LFSR[5];
        LFSR[3] <= LFSR[4];
        LFSR[2] <= LFSR[3];
        LFSR[1] <= LFSR[2];
        LFSR[0] <= LFSR[1];
        LFSR[8] <= feedback;
    end
endmodule
