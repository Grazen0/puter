`default_nettype none `timescale 1ns / 1ps

module spi_controller_tb ();
  reg clk, rst_n;
  always #5 clk = ~clk;

  reg [1:0] cmd;
  reg start;
  reg [7:0] data;
  wire ready;

  wire sclk;
  wire ss;
  reg miso;
  wire mosi;

  spi_controller spi (
      .clk  (clk),
      .rst_n(rst_n),

      .cmd  (cmd),
      .start(start),
      .data (data),
      .ready(ready),

      .sclk(sclk),
      .ss  (ss),
      .miso(miso),
      .mosi(mosi)
  );

  task send_cmd(input reg [1:0] cmd_in);
    begin
      cmd   = cmd_in;
      start = 1;
      #10 start = 0;
    end
  endtask

  initial begin
    $dumpvars(0, spi_controller_tb);

    clk   = 0;
    rst_n = 0;
    start = 0;

    #10 rst_n = 1;

    #20;

    send_cmd(2'b00);

    data = 8'b0010_1110;
    send_cmd(2'b10);

    @(posedge ready);
    #5;

    data = 8'b1100_1010;
    send_cmd(2'b10);

    @(posedge ready);
    #5;

    send_cmd(2'b01);

    #100 $finish();
  end
endmodule



















