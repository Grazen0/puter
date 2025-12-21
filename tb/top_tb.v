`default_nettype none `timescale 1ns / 1ps

module top_tb ();
  reg clk, rst_n, ps2_clk, ps2_data, rs_rx, sd_miso;
  always #5 clk = ~clk;

  wire [3:0] vga_red;
  wire [3:0] vga_green;
  wire [3:0] vga_blue;
  wire h_sync;
  wire v_sync;

  wire rs_tx;
  wire sd_sclk, sd_cs, sd_mosi;

  top top (
      .clk  (clk),
      .rst_n(rst_n),

      .ps2_clk (ps2_clk),
      .ps2_data(ps2_data),

      .vga_red  (vga_red),
      .vga_green(vga_green),
      .vga_blue (vga_blue),
      .h_sync   (h_sync),
      .v_sync   (v_sync),

      .rs_rx(rs_rx),
      .rs_tx(rs_tx),

      .sd_sclk(sd_sclk),
      .sd_cs  (sd_cs),
      .sd_miso(sd_miso),
      .sd_mosi(sd_mosi)
  );

  always @(posedge top.sys_clk) begin
    if (|top.puter.data_wenable && top.puter.data_addr == 32'h1000_0000) begin
      $write("%c", top.puter.data_wdata);
      $fflush();
    end
  end


  task send_scancode(input reg [7:0] scancode);
    integer i;
    reg parity;

    begin
      ps2_data = 0;
      #5 ps2_clk = 0;
      #5 ps2_clk = 1;

      parity = 1;

      for (i = 0; i < 8; i = i + 1) begin
        ps2_data = scancode[i];
        #5 ps2_clk = 0;
        #5 ps2_clk = 1;

        parity = parity ^ scancode[i];
      end

      ps2_data = parity;
      #5 ps2_clk = 0;
      #5 ps2_clk = 1;

      ps2_data = 1;
      #5 ps2_clk = 0;
      #5 ps2_clk = 1;
    end
  endtask

  initial begin
    $dumpvars(0, top_tb);

    ps2_clk = 1;

    clk = 1;
    rst_n = 0;
    #100 rst_n = 1;

    $display("");

    @(posedge sd_sclk);
    sd_miso = 1;

    repeat (4) begin
      @(negedge sd_sclk);
    end

    sd_miso = 0;

    #500_000 $finish();

    send_scancode(8'h1C);
    #50_000 send_scancode(8'hE0);
    #50_000 send_scancode(8'h75);

    #1_000_000;
    $display("");
    $finish();
  end
endmodule
