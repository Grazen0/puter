`default_nettype none `timescale 1ns / 1ps

module cpu_branch_target_buffer #(
    parameter XLEN = 32,
    parameter SET_WIDTH = 8
) (
    input wire clk,
    input wire rst_n,

    input wire [XLEN-1:0] update_addr,
    input wire [XLEN-1:0] update_target_addr,
    input wire            update,

    input  wire [XLEN-1:0] branch_addr,
    output wire            branch_hit,
    output wire [XLEN-1:0] branch_target_addr
);
  localparam TAG_WIDTH = XLEN - SET_WIDTH;
  localparam SETS = 2 ** SET_WIDTH;

  reg [XLEN-1:0] targets[0:SETS-1];
  reg [TAG_WIDTH-1:0] tags[0:SETS-1];
  reg [SETS-1:0] valid;

  wire [TAG_WIDTH-1:0] branch_tag, update_tag;
  wire [SET_WIDTH-1:0] branch_set, update_set;

  assign {branch_tag, branch_set} = branch_addr;
  assign {update_tag, update_set} = update_addr;

  assign branch_hit = valid[branch_set] && branch_tag == tags[branch_set];
  assign branch_target_addr = targets[branch_set];

  always @(posedge clk) begin
    if (!rst_n) begin
      valid <= {SETS{1'b0}};
    end else begin
      if (update) begin
        valid[update_set]   <= 1;
        tags[update_set]    <= update_tag;
        targets[update_set] <= update_target_addr;
      end
    end
  end
endmodule












