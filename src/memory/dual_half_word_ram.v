`default_nettype none `timescale 1ns / 1ps

module dual_half_word_ram #(
    parameter SIZE_HWORDS = 0,
    parameter ADDR_WIDTH  = $clog2(2 * SIZE_HWORDS)
) (
    input wire clk,

    input  wire [ADDR_WIDTH-1:0] addr_1,
    input  wire [          15:0] wdata_1,
    input  wire [           1:0] wenable_1,
    output wire [          15:0] rdata_1,

    input  wire [ADDR_WIDTH-1:0] addr_2,
    output wire [          15:0] rdata_2
);
  reg [15:0] data[0:SIZE_HWORDS-1];

  wire [ADDR_WIDTH-2:0] hword_addr_1, hword_addr_2;
  wire offset_1, offset_2;

  assign {hword_addr_1, offset_1} = addr_1;
  assign {hword_addr_2, offset_2} = addr_2;

  wire [15:0] wvalue_base = data[hword_addr_1];
  reg  [15:0] wvalue;

  always @(*) begin
    wvalue = wvalue_base;

    if (wenable_1[0]) wvalue[7+(8*offset_1)-:8] = wdata_1[7:0];
    if (wenable_1[1]) wvalue[15+(8*offset_1)-:8] = wdata_1[15:8];
  end

  always @(posedge clk) begin
    if (|wenable_1) data[hword_addr_1] <= wvalue;
  end

  assign rdata_1 = data[hword_addr_1] >> (8 * offset_1);
  assign rdata_2 = data[hword_addr_2] >> (8 * offset_2);
endmodule
