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

`timescale 10ns/10ns

module test_spi;

    wire miso = x;
    wire led1 = 0;
    wire led2 = 0;
    wire [7:0] dbg = 0;

    reg ss = 1;
    reg mosi = 1;
    reg sck = 0;

    integer seed, i;
    reg [7:0] value = 8'ha5;

    initial begin
        #20 ss = 0;
        for (i = 0; i < 8; i = i + 1) begin
            #40;
            mosi = value[7-i];                  // MSBFIRST
            #10;
            sck = 1;    //posedge - read mosi
            #50 sck = 0;    //negedge - send miso
        end
        #30  ss = 1;
        #200 $finish;
    end

    reg clk = 0;
    always #1 clk = !clk; // Make a regular pulsing clock.

    spi_slave spi1 (clk, ss, mosi, sck, miso, led1, led2, dbg[7:0] );

initial begin
        $dumpfile("spi.vcd");
        $dumpvars(0, test_spi);
        $monitor("At time %t, miso is %d", $time, miso);
end
endmodule
