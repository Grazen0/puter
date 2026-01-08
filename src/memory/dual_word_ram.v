`default_nettype none `timescale 1ns / 1ps

module dual_word_ram #(
    parameter SIZE_BYTES = 0,
    parameter SIZE_WORDS = SIZE_BYTES / 4,
    parameter ADDR_WIDTH = $clog2(SIZE_BYTES)
) (
    input wire clk,

    input  wire [ADDR_WIDTH-1:0] addr_1,
    input  wire [          31:0] wdata_1,
    input  wire [           3:0] wenable_1,
    output wire [          31:0] rdata_1,

    input  wire [ADDR_WIDTH-1:0] addr_2,
    output wire [          31:0] rdata_2
);
  reg [31:0] data[0:SIZE_WORDS-1];

  wire [ADDR_WIDTH-3:0] word_addr_1, word_addr_2;
  wire [1:0] offset_1, offset_2;

  assign {word_addr_1, offset_1} = addr_1;
  assign {word_addr_2, offset_2} = addr_2;

  wire [31:0] wvalue_base = data[word_addr_1];
  reg  [31:0] wvalue;

  always @(*) begin
    wvalue = wvalue_base;

    if (wenable_1[0]) wvalue[7+(8*offset_1)-:8] = wdata_1[7:0];
    if (wenable_1[1]) wvalue[15+(8*offset_1)-:8] = wdata_1[15:8];
    if (wenable_1[2]) wvalue[23+(8*offset_1)-:8] = wdata_1[23:16];
    if (wenable_1[3]) wvalue[31+(8*offset_1)-:8] = wdata_1[31:24];
  end

  always @(posedge clk) begin
    if (|wenable_1) data[word_addr_1] <= wvalue;
  end

  assign rdata_1 = data[word_addr_1] >> (8 * offset_1);
  assign rdata_2 = data[word_addr_2] >> (8 * offset_2);
endmodule
