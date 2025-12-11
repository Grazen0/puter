`default_nettype none `timescale 1ns / 1ps

module keyboard_controller_tb ();
  reg ps2_clk, ps2_data, rst_n;

  wire [7:0] data;
  wire valid;

  keyboard_controller keyboard_controller (
      .rst_n(rst_n),

      .ps2_clk (ps2_clk),
      .ps2_data(ps2_data),

      .data (data),
      .valid(valid)
  );

  initial begin
    $dumpvars(0, keyboard_controller_tb);

    ps2_clk = 1;
    rst_n   = 0;
    #5 rst_n = 1;

    #1 ps2_data = 0;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 1;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 0;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 1;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 1;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 0;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 0;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 1;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 0;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 0;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #1 ps2_data = 1;
    #1 ps2_clk = 0;
    #1 ps2_clk = 1;

    #10 $finish();
  end

endmodule
