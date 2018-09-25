//----------------------------------------------------------------------------
// Asynchronous serial transmitter and receiver
//
// Adapted with minor changes from: https://github.com/FPGAwars/FPGA-peripherals
//
//

//------------------------------------------
//-- (C) BQ. December 2015. Written by Juan Gonzalez (Obijuan)
//-- GPL license
//----------------------------------------------------------------------------
//-- Tested at the standard baudrates:
//-- 300, 600, 1200, 4800, 9600, 19200,
//-- 38400, 115200                      NB: Arduino, above SoftwareSerial max baud rate
//----------------------------------------------------------------------------
//-- Although this transmitter has been written from the scratch, it has been
//-- inspired by the one developed in the swapforth proyect by James Bowman
//--
//-- https://github.com/jamesbowman/swapforth
//--
//-----------------------------------------------------------------------------
//-- Constants for standard BAUDRATES, calculated from 16000000/BAUDRATE, rounded to integer:
`define B115200 137
`define B57600  277
`define B38400  417

`define B19200  833
`define B9600   1667
`define B4800   3333
`define B2400   6666
`define B1200   13333
`define B600    26667
`define B300    53333

//-- Top design
module serial #(parameter BAUDRATE = `B9600)(
         input wire clk,         //-- System clock
         input wire rx,          //-- Serial input
         output wire tx,         //-- Serial output
         output ledR,
         output ledG,
         output ledB);

wire rcv;               //-- Received character signal
wire [7:0] data;        //-- Received data
reg rstn = 0;           //-- Reset signal
wire ready;             //-- Transmitter ready signal

always @(posedge clk)   //-- Initialization
  rstn <= 1;

//-- Show the 3 least significant bits in the leds
always @(posedge clk) begin
  ledR = data[0];
  ledG = data[1];
  ledB = data[2];
end

uart_rx #(.BAUDRATE(BAUDRATE)) RX0 (.clk(clk), .rstn(rstn),   //-- Receiver unit instantation
                                    .rx(rx), .rcv(rcv), .data(data));
uart_tx #(.BAUDRATE(BAUDRATE)) TX0 ( .clk(clk), .rstn(rstn),  //-- Transmitter unit instantation
                                     .start(rcv), .data(data), .tx(tx), .ready(ready));
endmodule

//----------------------------------------------------------------------------------------
//-- baudgen module
//--
//-- INPUTS:
//--     -clk: System clock (12 MHZ in the iceStick board)
//--     -clk_ena: clock enable:
//--            1. Normal working: The squeare signal is generated
//--            0: stoped. Output always 0
//-- OUTPUTS:
//--     - clk_out: Output signal. Pulse width: 1 clock cycle. Output not registered
//--                It tells the uart_rx when to sample the next bit
//--                       __                                         __
//--   ____________________| |________________________________________| |_____________________
//--   |                  ->  <- 1 clock cycle   |
//--   <-------  Period ------------------------->
//--
//---------------------------------------------------------------------------------------
module baudgen_rx #(
         parameter BAUDRATE = `B115200  //-- Default baudrate
)(
         input wire rstn,         //-- Reset (active low)
         input wire clk,          //-- System clock
         input wire clk_ena,      //-- Clock enable
         output wire clk_out      //-- Bitrate Clock output
);

//-- Number of bits needed for storing the baudrate divisor
localparam N = $clog2(BAUDRATE);

//-- Value for generating the pulse in the middle of the period
localparam M2 = (BAUDRATE >> 1);

//-- Counter for implementing the divisor (it is a BAUDRATE module counter)
//-- (when BAUDRATE is reached, it start again from 0)
reg [N-1:0] divcounter = 0;

//-- Contador módulo M
always @(posedge clk)
  if (!rstn)
    divcounter <= 0;
  else if (clk_ena)
    //-- Normal working: counting. When the maximum count is reached, it starts from 0
    divcounter <= (divcounter == BAUDRATE - 1) ? 0 : divcounter + 1;
  else
    //-- Counter fixed to its maximum value
    //-- When it is resumed it start from 0
    divcounter <= BAUDRATE - 1;

//-- The output is 1 when the counter is in the middle of the period, if clk_ena is active
//-- It is 1 only for one system clock cycle
assign clk_out = (divcounter == M2) ? clk_ena : 0;

endmodule

module uart_rx #(                //-- Serial receiver unit module
         parameter BAUDRATE = `B115200   //-- Default baudrate
)(
         input wire clk,         //-- System clock (16MHz)
         input wire rstn,        //-- Reset (Active low)
         input wire rx,          //-- Serial data input
         output reg rcv,         //-- Data is available (1)
         output reg [7:0] data   //-- Data received
);

wire clk_baud; //-- Transmission clock

//-- Control signals
reg bauden;  //-- Enable the baud generator
reg clear;   //-- Clear the bit counter
reg load;    //-- Load the received character into the data register

//-------------------------------------------------------------------
//--     DATAPATH
//-------------------------------------------------------------------

//-- The serial input is registered in order to follow the
//-- synchronous design rules
reg rx_r;

always @(posedge clk)
  rx_r <= rx;

//-- Baud generator
baudgen_rx #(BAUDRATE) baudgen0 ( .rstn(rstn), .clk(clk), .clk_ena(bauden), .clk_out(clk_baud));

reg [3:0] bitc; //-- Bit counter

always @(posedge clk)
  if (clear)
    bitc <= 4'd0;
  else if (clear == 0 && clk_baud == 1)
    bitc <= bitc + 1;

reg [9:0] raw_data;     //-- Shift register for storing the received bits

always @(posedge clk)
  if (clk_baud == 1)
    raw_data <= {rx_r, raw_data[9:1]};

always @(posedge clk)   //-- Data register. Store the character received
  if (rstn == 0)
    data <= 0;
  else if (load)
    data <= raw_data[8:1];

//-------------------------------------------
//-- CONTROLLER  (Finite state machine)
//-------------------------------------------

//-- Receiver states
localparam IDLE = 2'd0;  //-- IDLEde reposo
localparam RECV = 2'd1;  //-- Receiving data
localparam LOAD = 2'd2;  //-- Storing the character received
localparam DAV = 2'd3;   //-- Data is available

//-- fsm states
reg [1:0] state;
reg [1:0] next_state;

always @(posedge clk)    //-- Transition between states
  if (!rstn)
    state <= IDLE;
  else
    state <= next_state;

always @(*) begin        //-- Control signal generation and next states

  //-- Default values
  next_state = state;    //-- Stay in the same state by default
  bauden = 0;
  clear = 0;
  load = 0;

  case(state)

    //-- Idle state
    //-- Remain in this state until a start bit is received in rx_r
    IDLE: begin
      clear = 1;
      rcv = 0;
      if (rx_r == 0)
        next_state = RECV;
    end

    //-- Receiving state
    //-- Turn on the baud generator and wait for the serial package to be received
    RECV: begin
      bauden = 1;
      rcv = 0;
      if (bitc == 4'd10)
        next_state = LOAD;
    end

    //-- Store the received character in the data register (1 cycle)
    LOAD: begin
      load = 1;
      rcv = 0;
      next_state = DAV;
    end

    //-- Data Available (1 cycle)
    DAV: begin
      rcv = 1;
      next_state = IDLE;
    end

    default:
      rcv = 0;
  endcase
end
endmodule

//----------------------------------------------------------------------------------------
//-- baudgen module
//--
//-- INPUTS:
//--     -clk: System clock (12 MHZ in the iceStick board)
//--     -clk_ena: clock enable:
//--            1. Normal working: The squeare signal is generated
//--            0: stoped. Output always 0
//-- OUTPUTS:
//--     - clk_out: Output signal. Pulse width: 1 clock cycle. Output not registered
//--                It tells the uart_tx when to transmit the next bit
//--      __                                                         __
//--   __| |________________________________________________________| |________________
//--   ->  <- 1 clock cycle
//--
//---------------------------------------------------------------------------------------
module baudgen_tx #(
          parameter BAUDRATE = `B115200  //-- Default baudrate
)(
          input wire rstn,              //-- Reset (active low)
          input wire clk,               //-- System clock
          input wire clk_ena,           //-- Clock enable
          output wire clk_out           //-- Bitrate Clock output
);

//-- Number of bits needed for storing the baudrate divisor
localparam N = $clog2(BAUDRATE);

//-- Counter for implementing the divisor (it is a BAUDRATE module counter)
//-- (when BAUDRATE is reached, it start again from 0)
reg [N-1:0] divcounter = 0;

always @(posedge clk)
  if (!rstn)
    divcounter <= 0;
  else if (clk_ena)
    //-- Normal working: counting. When the maximum count is reached, it starts from 0
    divcounter <= (divcounter == BAUDRATE - 1) ? 0 : divcounter + 1;
  else
    //-- Counter fixed to its maximum value
    //-- When it is resumed it start from 0
    divcounter <= BAUDRATE - 1;

//-- The output is 1 when the counter is 0, if clk_ena is active
//-- It is 1 only for one system clock cycle
assign clk_out = (divcounter == 0) ? clk_ena : 0;

endmodule

//--- Serial transmitter unit module
//--- TX output is not registered
module uart_tx #(
         parameter BAUDRATE = `B115200  //-- Default baudrate
)(
         input wire clk,        //-- System clock (16MHz)
         input wire rstn,       //-- Reset  (Active low)
         input wire start,      //-- Set to 1 for starting the transmission
         input wire [7:0] data, //-- Byte to transmit
         output reg tx,         //-- Serial data output
         output reg ready       //-- Transmitter ready (1) / busy (0)
);

wire clk_baud;      //-- Transmission clock
reg [3:0] bitc;     //-- Bitcounter
reg [7:0] data_r;   //-- Registered data

//--------- control signals
reg load;           //-- Load the shifter register / reset
reg baud_en;        //-- Enable the baud generator

//-------------------------------------
//-- DATAPATH
//-------------------------------------

//-- Register the input data
always @(posedge clk)
  if (start == 1 && state == IDLE)
    data_r <= data;

//-- 1 bit start + 8 bits datos + 1 bit stop
//-- Shifter register. It stored the frame to transmit:
//-- 1 start bit + 8 data bits + 1 stop bit
reg [9:0] shifter;

//-- When the control signal load is 1, the frame is loaded
//-- when load = 0, the frame is shifted right to send 1 bit,
//--   at the baudrate determined by clk_baud
//--  1s are introduced by the left
always @(posedge clk)
  if (rstn == 0)        //-- Reset
    shifter <= 10'b11_1111_1111;
  else if (load == 1)   //-- Load mode
    shifter <= {data_r,2'b01};
  else if (load == 0 && clk_baud == 1)  //-- Shift mode
    shifter <= {1'b1, shifter[9:1]};

//-- Sent bit counter
//-- When load (=1) the counter is reset
//-- When load = 0, the sent bits are counted (with the raising edge of clk_baud)
always @(posedge clk)
  if (!rstn)
    bitc <= 0;
  else if (load == 1)
    bitc <= 0;
  else if (load == 0 && clk_baud == 1)
    bitc <= bitc + 1;

//-- The less significant bit is transmited through tx
//-- It is a registed output, because tx is connected to an Asynchronous bus
//--  and the glitches should be avoided
always @(posedge clk)
  tx <= shifter[0];

//-- Baud generator
baudgen_tx #( .BAUDRATE(BAUDRATE)) BAUD0 ( .rstn(rstn), .clk(clk),
                                           .clk_ena(baud_en), .clk_out(clk_baud));

//------------------------------
//-- CONTROLLER
//------------------------------

//-- fsm states
localparam IDLE  = 0;  //-- Idle state
localparam START = 1;  //-- Start transmission
localparam TRANS = 2;  //-- Transmitting data

//-- Registers for storing the states
reg [1:0] state;
reg [1:0] next_state;

always @(posedge clk)   //-- Transition between states
  if (!rstn)
    state <= IDLE;
  else
    state <= next_state;

always @(*) begin       //-- Control signal generation and next states

  //-- Default values
  next_state = state;   //-- Stay in the same state by default
  load = 0;
  baud_en = 0;

  case (state)

    //-- Idle state
    //-- Remain in this state until start is 1
    IDLE: begin
      ready = 1;
      if (start == 1)
        next_state = START;
    end

    //-- 1 cycle long
    //-- turn on the baudrate generator and the load the shift register
    START: begin
      load = 1;
      baud_en = 1;
      ready = 0;
      next_state = TRANS;
    end

    //-- Stay here until all the bits have been sent
    TRANS: begin
      baud_en = 1;
      ready = 0;
      if (bitc == 11)
        next_state = IDLE;
    end

    default:
      ready = 0;
  endcase
end

endmodule
