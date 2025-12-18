`default_nettype none `timescale 1ns / 1ps

module cpu_branch_predictor #(
    parameter XLEN = 32,
    parameter CTR_WIDTH = 3,
    parameter BYTE_OFFSET = 2,
    parameter SET_WIDTH = 8,
    parameter N_WIDTH = 4
) (
    input wire clk,
    input wire rst_n,

    input wire [XLEN-1:0] update_addr,
    input wire            update_taken,
    input wire            update,

    input  wire [XLEN-1:0] addr,
    output reg             taken
);
  localparam N = 2 ** N_WIDTH;

  localparam CTR_MIN = 0;
  localparam CTR_MAX = {CTR_WIDTH{1'b1}};

  localparam TAG_WIDTH = XLEN - SET_WIDTH;
  localparam SETS = 2 ** SET_WIDTH;

  localparam INIT_TAKEN = {1'b1, {(CTR_WIDTH - 1) {1'b0}}};
  localparam INIT_NOT_TAKEN = {1'b0, {(CTR_WIDTH - 1) {1'b1}}};

  reg [CTR_WIDTH-1:0] counters[0:SETS-1] [0:N-1];
  reg [TAG_WIDTH-1:0] tags    [0:SETS-1] [0:N-1];
  reg                 valid   [0:SETS-1] [0:N-1];
  reg [  N_WIDTH-1:0] idx     [0:SETS-1];

  wire [TAG_WIDTH-1:0] tag, update_tag;
  wire [SET_WIDTH-1:0] set, update_set;

  assign {tag, set} = addr[XLEN-1:BYTE_OFFSET];
  assign {update_tag, update_set} = update_addr[XLEN-1:BYTE_OFFSET];

  reg update_hit;
  reg [N_WIDTH-1:0] update_idx;

  integer i, j;

  always @(*) begin
    taken = 0;
    update_hit = 0;
    update_idx = {N_WIDTH{1'bx}};

    for (i = 0; i < N; i = i + 1) begin
      if (valid[set][i] && tag == tags[set][i]) begin
        taken = counters[set][i][CTR_WIDTH-1];
      end

      if (valid[update_set][i] && update_tag == tags[update_set][i]) begin
        update_hit = 1;
        update_idx = i;
      end
    end
  end

  always @(posedge clk) begin
    if (!rst_n) begin
      for (i = 0; i < SETS; i = i + 1) begin
        idx[i] <= 0;

        for (j = 0; j < N; j = j + 1) begin
          valid[i][j] <= 0;
        end
      end
    end else begin
      if (update) begin
        if (update_hit) begin
          if (update_taken) begin
            if (counters[update_set][update_idx] != CTR_MAX)
              counters[update_set][update_idx] <= counters[update_set][update_idx] + 1;
          end else begin
            if (counters[update_set][update_idx] != CTR_MIN)
              counters[update_set][update_idx] <= counters[update_set][update_idx] - 1;
          end
        end else begin
          counters[update_set][idx[update_set]] <= update_taken ? INIT_TAKEN : INIT_NOT_TAKEN;
          valid[update_set][idx[update_set]]    <= 1;
          tags[update_set][idx[update_set]]     <= update_tag;
          idx[update_set]                       <= idx[update_set] + 1;
        end
      end
    end
  end
endmodule
