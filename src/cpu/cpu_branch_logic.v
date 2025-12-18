`default_nettype none `timescale 1ns / 1ps

`include "cpu_branch_logic.vh"
`include "cpu_control.vh"

module cpu_branch_logic #(
    parameter XLEN = 32
) (
    input wire jump,
    input wire [1:0] jump_src,
    input wire jump_target_hit,
    input wire [XLEN-1:0] pc_jump,
    input wire [XLEN-1:0] jump_target_addr,

    input wire branch,
    input wire [2:0] branch_cond,
    input wire branch_pred_taken,
    input wire exception,

    input wire alu_carry,
    input wire alu_overflow,
    input wire alu_zero,
    input wire alu_neg,

    output reg branch_cond_val,
    output reg [1:0] pc_src
);
  wire alu_lt = alu_neg ^ alu_overflow;

  wire jump_required = jump && (!jump_target_hit || pc_jump != jump_target_addr);

  wire branch_required = branch && branch_cond_val && !branch_pred_taken;
  wire branch_cancel = branch && !branch_cond_val && branch_pred_taken;

  always @(*) begin
    pc_src = `PC_SRC_PC_PLUS_4;

    case (branch_cond)
      `BRANCH_COND_EQ:  branch_cond_val = alu_zero;
      `BRANCH_COND_NE:  branch_cond_val = !alu_zero;
      `BRANCH_COND_LT:  branch_cond_val = alu_lt;
      `BRANCH_COND_GE:  branch_cond_val = !alu_lt;
      `BRANCH_COND_LTU: branch_cond_val = !alu_carry;
      `BRANCH_COND_GEU: branch_cond_val = alu_carry;
      default:          branch_cond_val = 1'bx;
    endcase

    if (exception) begin
      pc_src = `PC_SRC_MTVEC;
    end else if (branch_cancel) begin
      pc_src = `PC_SRC_PC_PLUS_4_E;
    end else if (jump_required || branch_required) begin
      pc_src = `PC_SRC_JUMP;
    end
  end
endmodule
