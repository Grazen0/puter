`default_nettype none `timescale 1ns / 1ps

module cache #(
    parameter XLEN = 32,
    parameter BYTE_OFFSET = 2,
    parameter SET_WIDTH = 6,
    parameter N_WIDTH = 1
) (
    input wire clk,
    input wire rst_n,

    input wire [XLEN-1:0] update_addr,
    input wire [XLEN-1:0] update_data,
    input wire            update,

    input  wire [XLEN-1:0] addr,
    output reg             hit,
    output reg  [XLEN-1:0] out_data
);
  localparam N = 2 ** N_WIDTH;

  localparam TAG_WIDTH = XLEN - BYTE_OFFSET - SET_WIDTH;
  localparam SETS = 2 ** SET_WIDTH;


  reg [     XLEN-1:0] data [0:SETS-1] [0:N-1];
  reg [TAG_WIDTH-1:0] tags [0:SETS-1] [0:N-1];
  reg                 valid[0:SETS-1] [0:N-1];
  reg [  N_WIDTH-1:0] idx  [0:SETS-1];

  wire [TAG_WIDTH-1:0] tag, update_tag;
  wire [SET_WIDTH-1:0] set, update_set;

  assign {tag, set} = addr[XLEN-1:BYTE_OFFSET];
  assign {update_tag, update_set} = update_addr[XLEN-1:BYTE_OFFSET];

  reg update_hit;

  integer i, j;

  always @(*) begin
    hit        = 0;
    update_hit = 0;
    out_data   = {XLEN{1'bx}};

    for (i = 0; i < N; i = i + 1) begin
      if (valid[set][i] && tag == tags[set][i]) begin
        hit = 1;
        out_data = data[set][i];
      end

      if (valid[update_set][i] && update_tag == tags[update_set][i]) begin
        update_hit = 1;
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
        for (i = 0; i < N; i = i + 1) begin
          if (valid[update_set][i] && update_tag == tags[update_set][i]) begin
            data[update_set][i] <= update_data;
          end
        end

        if (!update_hit) begin
          valid[update_set][idx[update_set]] <= 1;
          tags[update_set][idx[update_set]]  <= update_tag;
          data[update_set][idx[update_set]]  <= update_data;
          idx[update_set]                    <= idx[update_set] + 1;
        end
      end
    end
  end
endmodule












