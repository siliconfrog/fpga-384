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

//Binary to BCD conversion
module BCD(clk, Tens, Ones);
    input clk;
    output [3:0] Tens;
    output [3:0] Ones;

    reg [3:0] TensCalc;
    reg [3:0] OnesCalc;
    reg [27:0] counter = 0;
    reg [6:0] binary = 7'b0;

    assign Tens = TensCalc;
    assign Ones = OnesCalc;

    integer i;
    always @(binary) begin
        TensCalc = 4'd0;                // set 10's and 1's to 0
        OnesCalc = 4'd0;

        for (i = 6; i >= 0; i = i-1)
        begin
            if (TensCalc >= 5)          // add 3 to columns if >= 5
                TensCalc = TensCalc + 3;
            if (OnesCalc >= 5)
                OnesCalc = OnesCalc + 3;

            TensCalc = TensCalc << 1;   // and shift left one
            TensCalc[0] = OnesCalc[3];
            OnesCalc = OnesCalc << 1;
            OnesCalc[0] = binary[i];
        end
    end

    always @ (posedge clk) begin
        counter <= counter + 1;
        binary <= counter[27:21];
    end
endmodule
