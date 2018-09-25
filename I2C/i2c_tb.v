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

`timescale 1ns / 1ns

module test_I2C;
    wire SCL;
    wire SDA;
    wire [2:0] state;
    wire sendACK;
    wire LEDR;
    wire LEDG;
    wire LEDB;

    reg scl = 1;
    reg sda = 1;
    reg ledR = 0;
    reg ledG = 0;
    reg ledB = 0;
    parameter addr = 7'h24;
    parameter data = 8'h5A;

    initial begin
        #10

        #25 sda = 1'b0;     scl = 1'b1;
        #25 sda = 1'b0;     scl = 1'b0;      // START

        #22 sda = addr[6];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b6

        #22 sda = addr[5];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b5

        #22 sda = addr[4];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b4

        #22 sda = addr[3];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b3

        #22 sda = addr[2];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b2

        #22 sda = addr[1];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b1

        #22 sda = addr[0];  #3 scl = 1'b1;
        #25                 scl = 1'b0;      // b0

        #22 sda = 1'b0;     #3 scl = 1'b1;      // R/W bit
        #25                 scl = 1'b0;      // 0 write,  1=read

        #22 sda = 1'bz;     #3 scl = 1'b1;      // ADDR_ACK
        #25                 scl = 1'b0;


        #25 sda = data[7];  scl = 1'b1;      // DATA_READ
        #25                 scl = 1'b0;      // b7

        #25 sda = data[6];  scl = 1'b1;
        #25                 scl = 1'b0;      // b6

        #25 sda = data[5];  scl = 1'b1;
        #25                 scl = 1'b0;      // b5

        #25 sda = data[4];  scl = 1'b1;
        #25                 scl = 1'b0;      // b4

        #25 sda = data[3];  scl = 1'b1;
        #25                 scl = 1'b0;      // b3

        #25 sda = data[2];  scl = 1'b1;
        #25                 scl = 1'b0;      // b2

        #25 sda = data[1];  scl = 1'b1;
        #25                 scl = 1'b0;      // b1

        #25 sda = data[0];  scl = 1'b1;
        #25                 scl = 1'b0;      // b0

        #25 sda = 1'bz;     scl = 1'b1;      // DATA_ACK
        #25                 scl = 1'b0;

        #25 sda = 1'b1;     scl = 1'b1;      // STOP
        #25 sda = 1'b1;     scl = 1'b0;      // 
        #25 sda = 1'b1;     scl = 1'b1;      // 

        #20 $finish;
    end

    assign SCL = scl;
    assign SDA = sda;
    assign LEDR = ledR;
    assign LEDG = ledG;
    assign LEDB = ledB;
    reg clk = 0;
    always #1 clk = !clk;

    I2Cslave i2c1 (clk, SCL, SDA, state, sendACK, LEDR, LEDG, LEDB);


initial begin
        $dumpfile("I2C.vcd");
        $dumpvars(0, test_I2C);
        $monitor("At time %t, SDA is %d", $time, sda);
end
endmodule
