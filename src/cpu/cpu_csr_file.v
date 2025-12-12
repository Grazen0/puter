`default_nettype none `timescale 1ns / 1ps

`include "cpu_csr_file.vh"
`include "cpu_control.vh"

module cpu_csr_file #(
    parameter XLEN = 32
) (
    input wire clk,
    input wire rst_n,

    input wire [11:0] raddr,
    output reg [XLEN-1:0] rdata,

    input wire [    11:0] waddr,
    input wire [XLEN-1:0] wdata,
    input wire            wenable,

    input wire bubble_w,

    output reg [XLEN-1:0] mtvec,
    output reg [XLEN-1:0] mepc,

    input wire            exception_w,
    input wire [     1:0] exception_cause_w,
    input wire [XLEN-1:0] pc_w,

    output reg [1:0] priv
);
  reg [1:0] priv_next;

  reg [63:0] mstatus, mstatus_next;
  reg [XLEN-1:0] mie, mie_next;
  reg [XLEN-1:0] mtvec_next;

  reg [XLEN-1:0] mscratch, mscratch_next;
  reg [XLEN-1:0] mepc_next;
  reg [XLEN-1:0] mcause, mcause_next;
  reg [XLEN-1:0] mip, mip_next;

  reg [63:0] mcycle, mcycle_next;
  reg [63:0] minstret, minstret_next;

  always @(*) begin
    priv_next     = priv;

    mstatus_next  = mstatus;
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
        `CSR_MSTATUS:  mstatus_next[31:0] = wdata;
        `CSR_MSTATUSH: mstatus_next[63:32] = wdata;
        `CSR_MIE:      mie_next = wdata;
        `CSR_MTVEC:    mtvec_next = wdata;

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

    if (!bubble_w && !exception_w) begin
      minstret_next = minstret_next + 1;
    end

    if (exception_w) begin
      priv_next = `PRIV_M;
      mepc_next = pc_w;

      mstatus_next[12:11] = priv;

      case (exception_cause_w)
        `EXCAUSE_INSTR_ADDR_MISALIGNED: mcause_next = {1'b0, 31'd0};
        `EXCAUSE_ILLEGAL_INSTR:         mcause_next = {1'b0, 31'd2};
        `EXCAUSE_BREAKPOINT:            mcause_next = {1'b0, 31'd3};
        `EXCAUSE_ECALL:                 mcause_next = {1'b0, 31'd11};
        default:                        mcause_next = {(XLEN - 1) {1'bx}};
      endcase
    end

    // TODO: add mstatus logic when mret occurs

    // read-only mstatus fields
    mstatus_next[37]    <= 0;
    mstatus_next[36]    <= 0;
    mstatus_next[35:34] <= 2'd1;  // xlen = 32
    mstatus_next[33:32] <= 2'd1;  // xlen = 32
    mstatus_next[16:15] <= 0;

    case (raddr)
      `CSR_MSTATUS:  rdata = mstatus[31:0];
      `CSR_MSTATUSH: rdata = mstatus[63:32];
      `CSR_MIE:      rdata = mie;
      `CSR_MTVEC:    rdata = mtvec;

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
    if (!rst_n) begin
      priv           <= `PRIV_M;

      mstatus[38]    <= 0;
      mstatus[37]    <= 0;
      mstatus[36]    <= 0;
      mstatus[35:34] <= XLEN == 32 ? 2'd1 : 2'd2;
      mstatus[33:32] <= XLEN == 32 ? 2'd1 : 2'd2;
      mstatus[17]    <= 0;
      mstatus[16:15] <= 0;
      mstatus[12:11] <= `PRIV_M;

      mcycle         <= 0;
    end else begin
      priv     <= priv_next;

      mstatus  <= mstatus_next;
      mie      <= mie_next;
      mtvec    <= mtvec_next;

      mscratch <= mscratch_next;
      mepc     <= mepc_next;
      mcause   <= mcause_next;
      mip      <= mip_next;

      mcycle   <= mcycle_next;
      minstret <= minstret_next;
    end
  end

  initial begin
    mcycle   = 0;
    minstret = 0;
  end
endmodule

