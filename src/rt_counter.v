`default_nettype none `timescale 1ns / 1ps

module rt_counter (
    input wire sys_clk,  // TODO: use this clock for reads and writes
    input wire clk,
    input wire rst_n,

    input  wire        reg_sel,
    input  wire        h_sel,
    input  wire [31:0] wdata,
    input  wire        wenable,
    output reg  [31:0] rdata,

    output wire int_pending
);
  wire [1:0] addr = {reg_sel, h_sel};

  reg [63:0] mtime, mtime_next;
  reg [63:0] mtimecmp, mtimecmp_next;

  always @(*) begin
    mtime_next = mtime + 1;
    mtimecmp_next = mtimecmp;

    if (wenable) begin
      case (addr)
        2'b00: mtime_next[31:0] = wdata;
        2'b01: mtime_next[63:32] = wdata;
        2'b10: mtimecmp_next[31:0] = wdata;
        2'b11: mtimecmp_next[63:32] = wdata;
        default: begin
        end
      endcase
    end

    case (addr)
      2'b00: rdata = mtime[31:0];
      2'b01: rdata = mtime[63:32];
      2'b10: rdata = mtimecmp[31:0];
      2'b11: rdata = mtimecmp[63:32];
      default: begin
      end
    endcase
  end

  always @(posedge clk) begin
    if (!rst_n) begin
      mtime <= 0;
    end else begin
      mtime    <= mtime_next;
      mtimecmp <= mtimecmp_next;
    end
  end

  assign int_pending = mtime >= mtimecmp;
endmodule









