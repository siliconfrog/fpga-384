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

// 3 to 8 way decoder
module Decoder3to8(clk, in, out);
    input clk;
    input [3:0] in;
    output [7:0] out;

    reg [7:0] clkOut;

    assign out = clkOut;

    always @(posedge clk) begin
        case (in)
        4'b0000:     begin clkOut <= 8'b00000001; end
        4'b0001:     begin clkOut <= 8'b00000010; end
        4'b0010:     begin clkOut <= 8'b00000100; end
        4'b0011:     begin clkOut <= 8'b00001000; end
        4'b0100:     begin clkOut <= 8'b00010000; end
        4'b0101:     begin clkOut <= 8'b00100000; end
        4'b0110:     begin clkOut <= 8'b01000000; end
        4'b0111:     begin clkOut <= 8'b10000000; end
        4'b1000:     begin clkOut <= 8'b10000000; end
        4'b1001:     begin clkOut <= 8'b01000000; end
        4'b1010:     begin clkOut <= 8'b00100000; end
        4'b1011:     begin clkOut <= 8'b00010000; end
        4'b1100:     begin clkOut <= 8'b00001000; end
        4'b1101:     begin clkOut <= 8'b00000100; end
        4'b1110:     begin clkOut <= 8'b00000010; end
        4'b1111:     begin clkOut <= 8'b00000001; end
        endcase
    end

endmodule
