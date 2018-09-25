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
            _   _   _   _   _   _   _   _
SCK  ______| |_| |_| |_| |_| |_| |_| |_| |___

     ____                                  ________
SDA      |__X___X___X___X___X___X___X_____|

         ^ Start                        ^
           A6  A5  A4  A3  A2  A1  A0   ACK

*/

module I2Cslave(input clk, input SCL, inout SDA,
                output [2:0] stateO, output driveSDA,
                output ledR, output ledG, output ledB);

parameter I2C_ADR = 7'h24;              // Our 7-bit address

parameter START = 0,
          ADDR_READ_1 = 1,
          ADDR_READ_2 = 2,
          ADDR_ACK_1 = 3,
          ADDR_ACK_2 = 4,
          DATA_READ_1 = 5,
          DATA_READ_2 = 6,
          DATA_ACK_1 = 7,
          DATA_ACK_2 = 8,
          STOP = 9;
reg [2:0] state;
reg [2:0] bit;
reg [7:0] IOout;
reg sendACK = 1;
reg started = 0;

wire SDA_in;
wire SDA_out;
`ifdef SIMULATE
`else
SB_IO #(
    .PIN_TYPE(6'b 1010_01),
    .PULLUP(1'b 1),
) SDA_config (
    .PACKAGE_PIN(SDA),
    .OUTPUT_ENABLE(!driveSDA),
    .D_IN_0(SDA_in),
    .D_OUT_0(SDA_out)
);
`endif

//Synch SCL edge to the CPLD clock
reg [3:0] SCL_sync = 4'b0000;  
reg [3:0] SDA_sync = 4'b0000;
always @(posedge clk) SCL_sync <= { SCL_sync[2:0], SCL };
always @(posedge clk) SDA_sync <= { SDA_sync[2:0], SDA_in };
    
wire SCL_posedge = (SCL_sync == 4'b0111);  
wire SCL_negedge = (SCL_sync == 4'b1000);  

always @(negedge SDA_in) begin
    if (state == START) begin
        if (SCL == 1'b1) begin
            started = 1;
        end else begin
            started = 0;
        end
    end else begin
            started = 0;
    end
end


always @(posedge clk) begin
    case (state)
        START:          begin
                            bit = 3'b111;
                            sendACK = 1;
                            if (started == 1) begin
                                IOout = 8'b0;
                                state = ADDR_READ_1;
                            end
                        end
        ADDR_READ_1:    begin
                            if (SDA_in != I2C_ADR[bit-1]) begin
                                state = START;
                            end else begin
                                state = ADDR_READ_2;
                            end
                        end

        ADDR_READ_2:    begin
                            if (SCL_negedge) begin
                                if (bit == 3'b000) begin
                                    state = ADDR_ACK_1;
                                end
                            end
                            if (SCL_posedge) begin
                                bit <= bit - 1;
                                state = ADDR_READ_1;
                            end
                        end

        ADDR_ACK_1:     begin
                            sendACK = 0;
                            if (SCL_negedge == 1) begin
                                sendACK = 1;
                                state = ADDR_ACK_2;
                            end
                        end

        ADDR_ACK_2:     begin
                            if (SCL_posedge == 1) begin
                                bit = 3'b111;
                                state = DATA_READ_1;
                            end
                        end
            
        DATA_READ_1:    begin
                            IOout[bit] = SDA_in;
                            state = DATA_READ_2;
                        end

        DATA_READ_2:    begin
                            if (SCL_negedge) begin
                                if (bit == 3'b000) begin
                                    state = DATA_ACK_1;
                                end
                            end
                            if (SCL_posedge) begin
                                bit <= bit - 1;
                                state = DATA_READ_1;
                            end
                        end

        DATA_ACK_1:     begin
                            sendACK = 0;
                            if (SCL_negedge == 1) begin
                                sendACK = 1;
                                state = DATA_ACK_2;
                            end
                        end

        DATA_ACK_2:     begin
                            if (SCL_posedge == 1) begin
                                bit = 3'b111;
                                state = STOP;
                            end
                        end
            
        default:        state = START;
    endcase
end

assign stateO = state;
assign driveSDA = sendACK;
assign SDA_out = 0;
assign ledR = IOout[0];
assign ledG = IOout[1];
assign ledB = IOout[2];

endmodule
