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

// Pattern detect
module pattern(clk, dataIn, dataClk, dataEn, ledR, ledG, ledB);
    input clk, dataIn, dataClk, dataEn;
    output ledR, ledG, ledB;

    parameter patternLength = 64;
    reg [patternLength-1:0] pattern =
        patternLength'b0101011101001010101101011101111011101101010100010111100110000100;
    reg [7:0] patternBit = 8'b0;
    reg patternMatched = 0;

    reg dataClkPrev = 0;
    reg [2:0] stabilityCounter = 3'b0;

    assign ledR = !patternMatched;
    assign ledG =  patternMatched;
    assign ledB = dataIn;

    always @(posedge clk) begin
        if (dataEn == 0) begin
            if (dataClkPrev == 0 && dataClk == 1) begin     // clk rise
                // Count a few cycles to let the clock stabilise
                stabilityCounter <= stabilityCounter + 3'b1;
                if (stabilityCounter == 3'b0) begin
                    dataClkPrev = dataClk;
                    if (patternMatched == 0) begin
                        // Checking, and the next bit matches
                        if (dataIn == pattern[patternBit]) begin
                            patternBit <= patternBit + 1;
                            if (patternBit == patternLength-1) begin
                                patternMatched = 1;
                            end
                        end else begin
                            // Match failed - start again
                            patternBit = 0;
                        end
                    end
                end
            end else if (dataClkPrev == 1 && dataClk == 0) begin // clk fall
                // Count a few cycles to let the clock stabilise
                stabilityCounter <= stabilityCounter + 3'b1;
                if (stabilityCounter == 3'b0) begin
                    dataClkPrev = dataClk;
                end
            end
        end else begin
            patternBit = 0;
        end
    end
endmodule
