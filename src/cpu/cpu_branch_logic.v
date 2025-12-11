`default_nettype none `timescale 1ns / 1ps

`include "cpu_branch_logic.vh"
`include "cpu_control.vh"

module cpu_branch_logic (
    input wire jump,
    input wire [1:0] jump_src,
    input wire branch,
    input wire [2:0] branch_cond,
    input wire exception,

    input wire alu_carry,
    input wire alu_overflow,
    input wire alu_zero,
    input wire alu_neg,

    output reg [2:0] pc_src
);
  reg  branch_cond_val;

  wire alu_lt = alu_neg ^ alu_overflow;

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
    end else if (jump || (branch && branch_cond_val)) begin
      case (jump_src)
        `JUMP_SRC_PC_TARGET: pc_src = `PC_SRC_PC_TARGET;
        `JUMP_SRC_ALU:       pc_src = `PC_SRC_ALU;
        `JUMP_SRC_MTVEC:     pc_src = `PC_SRC_MTVEC;
        `JUMP_SRC_MEPC:      pc_src = `PC_SRC_MEPC;
        default:             pc_src = 2'bxx;
      endcase
    end
  end
endmodule
