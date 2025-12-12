`default_nettype none `timescale 1ns / 1ps

module synchronizer #(
    parameter STAGES = 2,
    parameter RESET_VALUE = 1'b0
) (
    input wire clk,
    input wire rst_n,

    input  wire in,
    output wire out
);
  reg [STAGES-1:0] stages;

  integer i;

  always @(posedge clk) begin
    if (!rst_n) begin
      for (i = 0; i < STAGES; i = i + 1) begin
        stages[i] <= RESET_VALUE;
      end
    end else begin
      stages[0] <= in;

      for (i = 1; i < STAGES; i = i + 1) begin
        stages[i] <= stages[i-1];
      end
    end
  end

  assign out = stages[STAGES-1];
endmodule
