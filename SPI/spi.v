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

/*
     _____                              ________
SS        |____________________________|
            _   _   _   _   _   _   _ 
SCK  ______| |_| |_| |_| |_| |_| |_| |_____

Data       X Y X Y X Y X Y X Y X Y X Y            MISO X   MOSI Y

Reflect the data received from MOSI back to MISO on the next transfer
*/
module spi_slave( clk, ss, mosi, sck, miso, ledR, ledG);
    input clk, ss, mosi, sck;
    output miso, ledR, ledG;

    reg [2:0] bit_ctr;
    reg [7:0] dataIn;
    reg [7:0] dataOut = 8'h0;
    reg misoVal = 0;
    reg sckPrev = 1;
    reg [1:0] stabilityCounter = 0;
    reg ssPrev = 1;

    always @(posedge clk) begin
        if (!ss) begin
            if (ssPrev == 1) begin                          // Prep MSB of MISO
                misoVal = dataOut[7];
                ssPrev = ss;
            end
            if (sckPrev == 0 && sck == 1) begin             // posedge in SCK
                // Count a few cycles to let the clock stabilise
                stabilityCounter <= stabilityCounter + 1;
                if (stabilityCounter == 0) begin
                    sckPrev = sck;
                    dataIn[bit_ctr] = mosi;                 // Sample data in
                    misoVal = dataOut[(bit_ctr+7)&7];       // Next bit to send out (1 bit behind)

                    // Transfer a bit that was received a few cycles ago
                    dataOut[(bit_ctr+4)&7] <= dataIn[(bit_ctr+4)&7];
                    // if (bit_ctr == 3'b000) begin ***dataIn complete***    end
                    //if (bit_ctr == 3'b001) begin //***dataOut = NextByte*** end
                    bit_ctr <= bit_ctr - 1;                 // MSBFIRST bitcounter
                end
            end else if (sckPrev == 1 && sck == 0) begin    // negedge in SCK
                sckPrev = sck;
            end
        end else begin 
            bit_ctr = 7;
            misoVal = 0;
            ssPrev  = 1;
        end
    end
    assign miso = misoVal;
    assign ledR = dataIn[7];
    assign ledG = dataIn[6];
endmodule
