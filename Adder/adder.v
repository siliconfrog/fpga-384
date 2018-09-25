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

// Half and Full Adder implementation
module Adder(clk, A, B, Cin, outA, outB, ledR, ledG, ledB);
    input clk, A, B, Cin;
    output outA, outB, ledR, ledG, ledB;

    reg Cout = 1;
    reg ps = 0;
    reg sum = 0;

    assign outA = sum;
    assign outB = Cout;

    // Also show on LEDs
    assign ledR = sum;
    assign ledG = Cout;
    assign ledB = 0;

    always @(posedge clk) begin
        ps = A ^ B;

        Cout = (A & B) | (ps & Cin);
        sum = ps ^ Cin;
    end
endmodule

//  Full Adder truth table:
//
//     A B Cin    PS   Sum Cout         PS   = A XOR B
//     0 0  0     0     0  0            Cout = (A AND B) OR (PS AND CIN)
//     0 0  1     0     1  0            Sum  = PS XOR CIN
//     0 1  0     1     1  0
//     0 1  1     1     0  1
//     1 0  0     1     1  0
//     1 0  1     1     0  1
//     1 1  0     0     0  1
//     1 1  1     0     1  1


// Half Adder:                          Sum = A ^ B       Carry = A & B
