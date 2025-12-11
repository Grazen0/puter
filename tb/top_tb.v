`default_nettype none `timescale 1ns / 1ps

module top_tb ();
  reg clk, rst_n, ps2_clk, ps2_data;
  always #5 clk = ~clk;

  wire [3:0] vga_red;
  wire [3:0] vga_green;
  wire [3:0] vga_blue;
  wire h_sync;
  wire v_sync;

  top top (
      .clk  (clk),
      .rst_n(rst_n),

      .ps2_clk (ps2_clk),
      .ps2_data(ps2_data),

      .vga_red  (vga_red),
      .vga_green(vga_green),
      .vga_blue (vga_blue),
      .h_sync   (h_sync),
      .v_sync   (v_sync)
  );

  always @(posedge top.sys_clk) begin
    if (|top.puter.data_wenable && top.puter.data_addr == 32'h1000_0000) begin
      $write("%c", top.puter.data_wdata);
    end
  end

  initial begin
    $dumpvars(0, top_tb);

    clk   = 1;
    rst_n = 0;
    #100 rst_n = 1;

    $display("");

    #1_000_000;
    $display("");
    $finish();
  end
endmodule
