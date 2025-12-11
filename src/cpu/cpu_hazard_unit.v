`default_nettype none `timescale 1ns / 1ps

`include "cpu_control.vh"
`include "cpu_hazard_unit.vh"
`include "cpu_branch_logic.vh"

module cpu_hazard_unit (
    input wire [4:0] rs1_d,
    input wire [4:0] rs2_d,
    input wire exception_d,

    input wire [ 4:0] rs1_e,
    input wire [ 4:0] rs2_e,
    input wire [ 4:0] rd_e,
    input wire [11:0] csrs_e,
    input wire [ 2:0] pc_src_e,
    input wire [ 2:0] result_src_e,
    input wire        csr_write_e,

    input wire        reg_write_m,
    input wire        csr_write_m,
    input wire [ 4:0] rd_m,
    input wire [11:0] csrs_m,

    input wire        reg_write_w,
    input wire        csr_write_w,
    input wire [ 4:0] rd_w,
    input wire [11:0] csrs_w,

    output reg [1:0] forward_a_e,
    output reg [1:0] forward_b_e,
    output reg [1:0] forward_csr_e,

    output wire stall_f,
    output wire stall_d,

    output wire flush_d,
    output wire flush_e
);
  always @(*) begin
    forward_a_e   = `FORWARD_NONE;
    forward_b_e   = `FORWARD_NONE;
    forward_csr_e = `FORWARD_NONE;

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

    // TODO: ignore when reading a read-only CSR
    if (csrs_e == csrs_m && csr_write_m) begin
      forward_csr_e = `FORWARD_MEMORY;
    end else if (csrs_e == csrs_w && csr_write_w) begin
      forward_csr_e = `FORWARD_WRITEBACK;
    end
  end

  wire lw_stall = result_src_e == `RESULT_SRC_DATA && (rs1_d == rd_e || rs2_d == rd_e);
  wire exception_stall = exception_d && (csr_write_e || csr_write_m);

  assign stall_f = lw_stall || exception_stall;
  assign stall_d = lw_stall || exception_stall;

  assign flush_d = pc_src_e != `PC_SRC_PC_PLUS_4;
  assign flush_e = lw_stall || exception_stall || pc_src_e != `PC_SRC_PC_PLUS_4;
endmodule
