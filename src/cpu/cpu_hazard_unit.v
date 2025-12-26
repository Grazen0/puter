`default_nettype none `timescale 1ns / 1ps

`include "cpu_control.vh"
`include "cpu_hazard_unit.vh"
`include "cpu_branch_logic.vh"

module cpu_hazard_unit (
    input wire [4:0] rs1_d,
    input wire [4:0] rs2_d,
    input wire       exception_d,

    input wire [4:0] rs1_e,
    input wire [4:0] rs2_e,
    input wire [4:0] rd_e,
    input wire [1:0] pc_src_e,
    input wire [2:0] result_src_e,
    input wire       csr_write_e,
    input wire       exception_e,

    input wire        reg_write_m,
    input wire        csr_write_m,
    input wire [ 4:0] rd_m,
    input wire [11:0] csrs_m,
    input wire        exception_m,

    input wire        reg_write_w,
    input wire        csr_write_w,
    input wire [ 4:0] rd_w,
    input wire [11:0] csrs_w,

    output reg [1:0] forward_a_e,
    output reg [1:0] forward_b_e,

    output reg stall_f,
    output reg stall_d,

    output reg flush_d,
    output reg flush_e,
    output reg flush_m,

    input wire int_ack
);
  always @(*) begin
    forward_a_e = `FORWARD_NONE;
    forward_b_e = `FORWARD_NONE;

    if (rs1_e == rd_m && reg_write_m && rs1_e != 0) begin
      forward_a_e = `FORWARD_MEMORY;
    end else if (rs1_e == rd_w && reg_write_w && rs1_e != 0) begin
      forward_a_e = `FORWARD_WRITEBACK;
    end

    if (rs2_e == rd_m && reg_write_m && rs2_e != 0) begin
      forward_b_e = `FORWARD_MEMORY;
    end else if (rs2_e == rd_w && reg_write_w && rs2_e != 0) begin
      forward_b_e = `FORWARD_WRITEBACK;
    end
  end

  // NOTE: jump_flush, lw_stall and csr_stall are mutually exclusive. The
  // reason why is left as an exercise for the reader.

  // occurs when the pc jumps to something other than pc + 4
  wire jump_flush = pc_src_e != `PC_SRC_PC_PLUS_4;

  // occurs when an instruction reads a register right after reading memory
  // into it
  wire lw_stall = result_src_e == `RESULT_SRC_DATA && (rs1_d == rd_e | rs2_d == rd_e);

  // occurs when an instruction wants to write to the csr file. we could
  // forward, but there's way too many uses of each csr (not just by software
  // reads), and it's more complicated to handle each forward case.
  // NOTE: needs updating if there is ever another situation where a csr is
  // written to but wihout a jump (unlike an mret or an exception, which do
  // write to csr but also jump, which flushes instructions and does not
  // require stalling)
  //
  // NOTE: the exception checks might not be necessary when i add the unit to
  // turn off write signals on an illegal instruction
  wire csr_stall = (csr_write_e & ~exception_e) | (csr_write_m & ~exception_m);

  always @(*) begin
    if (int_ack) begin
      flush_d = 1;
      flush_e = 1;
      flush_m = 1;

      stall_f = 0;
      stall_d = 0;
    end else begin
      flush_d = jump_flush;
      flush_e = jump_flush | lw_stall | csr_stall;
      flush_m = 0;

      stall_f = lw_stall | csr_stall;
      stall_d = lw_stall | csr_stall;
    end
  end
endmodule
