`default_nettype none `timescale 1ns / 1ps

module cpu_branch_predictor #(
    parameter XLEN = 32,
    parameter CTR_WIDTH = 3,
    parameter BYTE_OFFSET = 2,
    parameter SET_WIDTH = 8
) (
    input wire clk,
    input wire rst_n,

    input wire [XLEN-1:0] update_addr,
    input wire            update_taken,
    input wire            update,

    input  wire [XLEN-1:0] branch_addr,
    output wire            branch_taken
);
  localparam CTR_MIN = 0;
  localparam CTR_MAX = {CTR_WIDTH{1'b1}};

  localparam TAG_WIDTH = XLEN - SET_WIDTH;
  localparam SETS = 2 ** SET_WIDTH;

  reg [CTR_WIDTH-1:0] counters[0:SETS-1];
  reg [TAG_WIDTH-1:0] tags[0:SETS-1];
  reg [SETS-1:0] valid;

  wire [TAG_WIDTH-1:0] branch_tag, update_tag;
  wire [SET_WIDTH-1:0] branch_set, update_set;

  assign {branch_tag, branch_set} = branch_addr[XLEN-1:BYTE_OFFSET];
  assign {update_tag, update_set} = update_addr[XLEN-1:BYTE_OFFSET];

  wire branch_hit = valid[branch_set] && branch_tag == tags[branch_set];
  wire update_hit = valid[update_set] && update_tag == tags[update_set];

  assign branch_taken = branch_hit && counters[branch_set][CTR_WIDTH-1];

  always @(posedge clk) begin
    if (!rst_n) begin
      valid <= {SETS{1'b0}};
    end else begin
      if (update) begin
        if (update_hit) begin
          if (update_taken) begin
            if (counters[update_set] != CTR_MAX) counters[update_set] <= counters[update_set] + 1;
          end else if (!update_taken) begin
            if (counters[update_set] != CTR_MIN) counters[update_set] <= counters[update_set] - 1;
          end
        end else begin
          if (update_taken) begin
            counters[update_set] <= {1'b1, {(CTR_WIDTH - 1) {1'b0}}};
          end else begin
            counters[update_set] <= {1'b0, {(CTR_WIDTH - 1) {1'b1}}};
          end
        end

        valid[update_set] <= 1;
        tags[update_set]  <= update_tag;
      end
    end
  end
endmodule
