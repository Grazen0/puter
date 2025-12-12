`timescale 1ns / 1ns `default_nettype none `timescale 1ns / 1ps

module cpu_tb ();
  reg clk, rst_n;
  always #5 clk = ~clk;

  wire [31:0] instr_addr;
  wire [31:0] data_addr;
  wire [31:0] data_wdata;
  wire [ 3:0] data_wenable;

  wire [31:0] instr_rdata;
  wire [31:0] rom_rdata;

  dual_word_rom #(
      .SOURCE_FILE("/home/jdgt/Code/utec/arqui/puter/build/firmware/firmware.mem")
  ) rom (
      .addr_1 (instr_addr[14:0]),
      .rdata_1(instr_rdata),

      .addr_2 (data_addr[14:0]),
      .rdata_2(rom_rdata)
  );

  wire [31:0] ram_rdata;

  word_ram ram (
      .clk(clk),

      .addr_1   (data_addr[14:0]),
      .wdata_1  (data_wdata),
      .wenable_1(data_wenable & {4{data_addr[31]}}),
      .rdata_1  (ram_rdata)
  );

  wire [31:0] data_rdata = data_addr[31] ? ram_rdata : rom_rdata;

  cpu cpu (
      .clk  (clk),
      .rst_n(rst_n),

      .instr_addr(instr_addr),
      .instr_data(instr_rdata),

      .data_addr   (data_addr),
      .data_wdata  (data_wdata),
      .data_wenable(data_wenable),
      .data_rdata  (data_rdata)
  );

  always @(posedge clk) begin
    #1;
    if (|data_wenable && data_addr == 32'h1000_0000) begin
      $write("%c", data_wdata);
      $fflush();
    end
  end

  initial begin
    $dumpvars(0, cpu_tb);

    clk   = 0;
    rst_n = 0;
    #10 rst_n = 1;

    $display("");

    #100_000;
    $display("");
    $finish();
  end
endmodule
