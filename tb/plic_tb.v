`default_nettype none `timescale 1ns / 1ps

module plic_tb ();
  reg clk, rst_n;
  always #5 clk = ~clk;

  localparam PORTS = 4;

  reg [PORTS-1:0] int_signal;
  reg [PORTS-1:0] int_claim;

  wire int_pending;
  wire [1:0] int_id;

  plic plic (
      .clk  (clk),
      .rst_n(rst_n),

      .int_signal(int_signal),
      .int_claim (int_claim),

      .out_int_pending(int_pending),
      .out_int_id     (int_id)
  );

  initial begin
    $dumpvars(0, plic_tb);

    clk = 0;
    rst_n = 0;

    int_signal = 4'b0000;
    int_claim = 4'b0000;
    plic.int_enable = 4'b1111;
    plic.int_priority[0] = 3'd3;
    plic.int_priority[1] = 3'd2;
    plic.int_priority[2] = 3'd4;
    plic.int_priority[3] = 3'd1;

    #10 rst_n = 1;
    #30 int_signal = 4'b1111;

    @(posedge int_pending);
    #20 int_claim = 4'b0100;
    #20 int_claim = 4'b0001;
    #30 int_claim = 4'b0010;
    #30 int_claim = 4'b1000;

    #80 $finish();
  end
endmodule
