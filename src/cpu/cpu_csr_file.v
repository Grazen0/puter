`default_nettype none

`include "cpu_csr_file.vh"

module cpu_csr_file (
    input wire clk,
    input wire rst_n,

    input  wire [11:0] raddr,
    output reg  [31:0] rdata,

    input wire [11:0] waddr,
    input wire [31:0] wdata,
    input wire wenable,

    input wire bubble_w,

    output reg [31:0] mtvec
);
  reg [31:0] mie, mie_next;
  reg [31:0] mtvec_next;

  reg [31:0] mscratch, mscratch_next;
  reg [31:0] mepc, mepc_next;
  reg [31:0] mcause, mcause_next;
  reg [31:0] mip, mip_next;

  reg [63:0] mcycle, mcycle_next;
  reg [63:0] minstret, minstret_next;

  always @(*) begin
    mie_next      = mie;
    mtvec_next    = mtvec;

    mscratch_next = mscratch;
    mepc_next     = mepc;
    mcause_next   = mcause;
    mip_next      = mip;

    mcycle_next   = mcycle;
    minstret_next = minstret;

    if (wenable) begin
      case (waddr)
        `CSR_MIE:   mie_next = wdata;
        `CSR_MTVEC: mtvec_next = wdata;

        `CSR_MSCRATCH: mscratch_next = wdata;
        `CSR_MEPC:     mepc_next = wdata;
        `CSR_MCAUSE:   mcause_next = wdata;
        `CSR_MIP:      mip_next = wdata;

        `CSR_MCYCLE:    mcycle_next[31:0] = wdata;
        `CSR_MCYCLEH:   mcycle_next[63:32] = wdata;
        `CSR_MINSTRET:  minstret_next[31:0] = wdata;
        `CSR_MINSTRETH: minstret_next[63:32] = wdata;
        default: begin
        end
      endcase
    end

    mcycle_next = mcycle_next + 1;

    if (!bubble_w) begin
      minstret_next = minstret_next + 1;
    end

    case (raddr)
      `CSR_MIE:   rdata = mie;
      `CSR_MTVEC: rdata = mtvec;

      `CSR_MSCRATCH: rdata = mscratch;
      `CSR_MEPC:     rdata = mepc;
      `CSR_MCAUSE:   rdata = mcause;
      `CSR_MIP:      rdata = mip;

      `CSR_MCYCLE:    rdata = mcycle[31:0];
      `CSR_MCYCLEH:   rdata = mcycle[63:32];
      `CSR_MINSTRET:  rdata = minstret[31:0];
      `CSR_MINSTRETH: rdata = minstret[63:32];

      default: rdata = {32{1'bx}};
    endcase
  end

  always @(posedge clk) begin
    mie      <= mie_next;
    mtvec    <= mtvec_next;

    mscratch <= mscratch_next;
    mepc     <= mepc_next;
    mcause   <= mcause_next;
    mip      <= mip_next;

    mcycle   <= mcycle_next;
    minstret <= minstret_next;
  end

`ifdef IVERILOG
  initial begin
    mcycle   = 0;
    minstret = 0;
  end
`endif
endmodule

