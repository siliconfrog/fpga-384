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

// PWM on all LEDs
module top (clk, ledR, ledG, ledB);
	input clk;
	output ledR, ledG, ledB;

	reg rledR;
	reg rledG;
	reg rledB;

	assign ledR = rledR;
	assign ledG = rledG;
	assign ledB = rledB;

	reg [11:0] counter = 12'b0;
	reg [11:0] trigger = 12'b0;
	reg [11:0] increment = 12'b1;

	always @ (posedge clk) begin
		counter <= counter + 1;
		
		if( counter == 0 ) begin
			trigger <= trigger + increment;
			
			if( trigger == 12'b111111111110 ) begin
				increment <= 12'b111111111111;
			end
			
			if( trigger == 12'b0 ) begin
				increment <= 12'b1;
			end
		end
		
		if( counter > trigger ) begin
			rledR <= 1;
			rledG <= 0;
			rledB <= 1;
		end else begin
			rledR <= 0;
			rledG <= 1;
			rledB <= 0;
		end
	end
	
endmodule
