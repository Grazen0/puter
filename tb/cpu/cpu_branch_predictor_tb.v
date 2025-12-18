`default_nettype none `timescale 1ns / 1ps

module cpu_branch_predictor_tb ();
  reg clk, rst_n;
  always #5 clk = ~clk;

  reg [31:0] update_addr;
  reg update_taken;
  reg update;

  reg [31:0] branch_addr;
  wire branch_take;

  cpu_branch_predictor branch_predictor (
      .clk  (clk),
      .rst_n(rst_n),

      .update_addr (update_addr),
      .update_taken(update_taken),
      .update      (update),

      .addr (branch_addr),
      .taken(branch_take)
  );

  initial begin
    $dumpvars(0, cpu_branch_predictor_tb);

    clk = 0;
    rst_n = 0;

    branch_addr = 32'h1234_0000;

    #10 rst_n = 1;

    update_addr = 32'h1234_0000;
    update_taken = 0;
    update = 1;
    #20 update = 0;

    update_taken = 1;
    update = 1;
    #40 update = 0;

    #100 $finish();
  end
endmodule
