`default_nettype none `timescale 1ns / 1ps

module plic_gateway (
    input wire clk,
    input wire rst_n,

    input  wire int_signal,
    output wire irq
);
  wire int_sync;
  reg  int_sync_prev;

  synchronizer synchronizer (
      .clk  (clk),
      .rst_n(rst_n),

      .in (int_signal),
      .out(int_sync)
  );

  always @(posedge clk) begin
    int_sync_prev <= int_sync;
  end

  assign irq = int_sync & ~int_sync_prev;  // posedge
endmodule

module plic #(
    parameter PORTS = 4,
    parameter ID_WIDTH = $clog2(PORTS),
    parameter PRIORITY_WIDTH = $clog2(PORTS + 1)
) (
    input wire clk,
    input wire rst_n,

    input wire [PORTS-1:0] int_signal,
    input wire [PORTS-1:0] int_claim,

    output wire out_int_pending,
    output wire [ID_WIDTH-1:0] out_int_id
);
  wire [PORTS-1:0] irq;

  generate
    genvar i;
    for (i = 0; i < PORTS; i = i + 1) begin : g_gateway
      plic_gateway gateway (
          .clk  (clk),
          .rst_n(rst_n),

          .int_signal(int_signal[i]),
          .irq       (irq[i])
      );
    end
  endgenerate

  reg [PRIORITY_WIDTH-1:0] int_threshold;
  reg [PORTS-1:0] int_pending;
  reg [PORTS-1:0] int_enable;
  reg [PORTS-1:0][PRIORITY_WIDTH-1:0] int_priority;

  integer i2;

  always @(posedge clk) begin
    if (!rst_n) begin
      int_pending   <= {PORTS{1'b0}};
      int_enable    <= {PORTS{1'b0}};  // TODO: set to 0 on reset
      int_threshold <= 0;
    end else begin
      for (i2 = 0; i2 < PORTS; i2 = i2 + 1) begin
        if (irq[i2]) int_pending[i2] <= 1;
        else if (int_claim[i2]) int_pending[i2] <= 0;
      end
    end
  end

  wire [PORTS:0][PRIORITY_WIDTH-1:0] max_priority;
  wire [PORTS:0][ID_WIDTH-1:0] max_id;

  assign max_priority[0] = 0;
  assign max_id[0]       = 0;

  generate
    for (i = 0; i < PORTS; i = i + 1) begin : g_priority_logic
      wire int_cond = int_enable[i] && int_pending[i] && int_priority[i] > max_priority[i];

      assign max_priority[i+1] = int_cond ? int_priority[i] : max_priority[i];
      assign max_id[i+1]       = int_cond ? i : max_id[i];
    end
  endgenerate

  assign out_int_pending = max_priority[PORTS] > int_threshold;
  assign out_int_id = max_id[PORTS];
endmodule
