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

// Multiplex a 4-digit 7-segment display

module top (clk, segCOM, segments);
    input clk;
    output [3:0] segCOM;            // Common cathode
    output [6:0] segments;          // 7-segment segments

                        // g d c a f b e
    localparam segA = 7'b0001000;   // Segment bit - depends upon wiring pin
    localparam segB = 7'b0000010;
    localparam segC = 7'b0010000;
    localparam segD = 7'b0100000;
    localparam segE = 7'b0000001;
    localparam segF = 7'b0000100;
    localparam segG = 7'b1000000;

    reg [20:0] counter = 0;
    reg [3:0] digs     = 4'b1111;   // Digit commons
    reg [6:0] segs     = 7'b0;      // Segments
    reg [3:0] n        = 4'b1111;   // Current digit displaying

    reg [3:0] numThou  = 4'd1;      // Register for thousands
    reg [3:0] numHund  = 4'd2;      // Register for hundreds
    reg [3:0] numTens  = 4'd3;      // Register for tens
    reg [3:0] numUnit  = 4'd4;      // Register for units
    reg [1:0] toDisp   = 0;         // Register for which digit to display

    assign segCOM = digs;
    assign segments = segs;

    always @ (posedge clk) begin
        counter <= counter + 16;
        if (counter == 0) begin     // Increment the number in bcd
            numUnit <= numUnit + 1;
            if (numUnit >= 4'b1001) begin       // rollover digit...
                numUnit <= 4'b0000;
                numTens <= numTens + 1;
                if (numTens >= 4'b1001) begin
                    numTens <= 4'b0000;
                    numHund <= numHund + 1;
                    if (numHund >= 4'b1001) begin
                        numHund <= 4'b0000;
                        numThou <= numThou + 1;
                        if (numThou >= 4'b1001) begin
                            numThou <= 4'b0000;
                        end
                    end
                end
            end
        end
        if (counter[12:0] == 13'b0) begin       // Multiplex quickly the digits
            toDisp <= toDisp + 1;
            case (toDisp)
            0: begin digs = 4'b0111; n <= numUnit; end
            1: begin digs = 4'b1011; n <= numTens; end
            2: begin digs = 4'b1101; n <= numHund; end
            3: begin digs = 4'b1110; n <= numThou; end

            default: begin digs = 4'b0111; n <= numUnit; end
            endcase
        end

        // Construct the segments from the number to be displayed
        case (n)
            0: segs = segA | segB | segC | segD | segE | segF       ;
            1: segs =        segB | segC                            ;
            2: segs = segA | segB |        segD | segE |        segG;
            3: segs = segA | segB | segC | segD |               segG;
            4: segs =        segB | segC |               segF | segG;
            5: segs = segA |        segC | segD        | segF | segG;
            6: segs = segA |        segC | segD | segE | segF | segG;
            7: segs = segA | segB | segC                            ;
            8: segs = segA | segB | segC | segD | segE | segF | segG;
            9: segs = segA | segB | segC | segD        | segF | segG;
            default:
               segs = 0;
        endcase
    end
endmodule
