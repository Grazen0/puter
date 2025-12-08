`default_nettype none

module word_ram #(
    parameter SIZE_WORDS  = 2 ** 13,
    parameter SOURCE_FILE = "",
    parameter ADDR_WIDTH  = $clog2(4 * SIZE_WORDS)
) (
    input wire clk,

    input  wire [ADDR_WIDTH-1:0] addr_1,
    input  wire [          31:0] wdata_1,
    input  wire [           3:0] wenable_1,
    output wire [          31:0] rdata_1
);
  reg [31:0] data[0:SIZE_WORDS-1];

  wire [29:0] word_addr_1 = addr_1[ADDR_WIDTH-1:2];
  wire [1:0] offset_1 = addr_1[1:0];

  reg [31:0] wvalue;

  always @(*) begin
    wvalue = data[word_addr_1];

    if (wenable_1[0]) wvalue[7+(8*offset_1)-:8] = wdata_1[7:0];
    if (wenable_1[1]) wvalue[15+(8*offset_1)-:8] = wdata_1[15:8];
    if (wenable_1[2]) wvalue[23+(8*offset_1)-:8] = wdata_1[23:16];
    if (wenable_1[3]) wvalue[31+(8*offset_1)-:8] = wdata_1[31:24];
  end

  always @(posedge clk) begin
    if (|wenable_1) data[word_addr_1] <= wvalue;
  end

  assign rdata_1 = data[word_addr_1] >> (8 * offset_1);

  initial begin
    if (SOURCE_FILE != "") begin
      $readmemh(SOURCE_FILE, data);
    end
  end
endmodule
