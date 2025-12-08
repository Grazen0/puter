`default_nettype none

`include "cpu_alu.vh"
`include "cpu_control.vh"
`include "cpu_imm_extend.vh"

module cpu_control (
    input wire [6:0] op,
    input wire [2:0] funct3,
    input wire [6:0] funct7,

    output reg       reg_write,
    output reg [1:0] result_src,
    output reg [3:0] mem_write,
    output reg       jump,
    output reg       branch,
    output reg [3:0] alu_control,
    output reg       alu_src_b,
    output reg [2:0] imm_src,
    output reg [2:0] data_ext_control,
    output reg       jump_src,
    output reg [2:0] branch_cond,
    output reg       illegal_instr
);
  always @(*) begin
    reg_write     = 0;
    result_src    = `RESULT_SRC_ALU;
    mem_write     = 4'b0000;
    jump          = 0;
    branch        = 0;
    alu_control   = 4'bxxxx;
    alu_src_b     = 1'bx;
    imm_src       = 3'bxxx;
    jump_src      = 1'bx;
    branch_cond   = 3'bxxx;
    illegal_instr = 0;

    case (op)
      7'b0000011: begin  // load
        imm_src          = `IMM_SRC_I;
        alu_src_b        = `ALU_SRC_B_IMM;
        alu_control      = `ALU_ADD;

        data_ext_control = funct3;

        result_src       = `RESULT_SRC_DATA;
        reg_write        = 1;
      end
      7'b0010011: begin  // alu immediate
        imm_src     = `IMM_SRC_I;
        alu_src_b   = `ALU_SRC_B_IMM;
        alu_control = {funct7[5] & (funct3 == 3'b101), funct3};

        result_src  = `RESULT_SRC_ALU;
        reg_write   = 1;
      end
      7'b0010111: begin  // auipc
        imm_src    = `IMM_SRC_U;
        result_src = `RESULT_SRC_PC_TARGET;
        reg_write  = 1;
      end
      7'b0100011: begin  // store
        imm_src     = `IMM_SRC_S;
        alu_src_b   = `ALU_SRC_B_IMM;
        alu_control = `ALU_ADD;

        case (funct3)
          3'b000:  mem_write = 4'b0001;
          3'b001:  mem_write = 4'b0011;
          3'b010:  mem_write = 4'b1111;
          default: mem_write = 4'b0000;
        endcase
      end
      7'b0110011: begin  // alu register
        imm_src     = `IMM_SRC_I;
        alu_src_b   = `ALU_SRC_B_RS2;
        alu_control = {funct7[5], funct3};

        result_src  = `RESULT_SRC_ALU;
        reg_write   = 1;
      end
      7'b0110111: begin  // lui
        imm_src     = `IMM_SRC_U;
        alu_src_b   = `ALU_SRC_B_IMM;
        alu_control = `ALU_PASS_B;
        result_src  = `RESULT_SRC_ALU;
        reg_write   = 1;
      end
      7'b1100011: begin  // branch
        imm_src     = `IMM_SRC_B;
        alu_src_b   = `ALU_SRC_B_RS2;
        alu_control = `ALU_SUB;
        branch      = 1;
        branch_cond = funct3;
        jump_src    = `JUMP_SRC_PC_TARGET;
      end
      7'b1100111: begin  // jalr
        imm_src     = `IMM_SRC_I;
        alu_src_b   = `ALU_SRC_B_IMM;
        alu_control = `ALU_ADD;
        jump        = 1;
        jump_src    = `JUMP_SRC_ALU;

        result_src  = `RESULT_SRC_PC_PLUS_4;
        reg_write   = 1;
      end
      7'b1101111: begin  // jal
        imm_src    = `IMM_SRC_J;
        jump       = 1;
        jump_src   = `JUMP_SRC_PC_TARGET;
        result_src = `RESULT_SRC_PC_PLUS_4;
        reg_write  = 1;
      end
      default: begin
        // TODO: trigger interrupt
        illegal_instr = 1;
      end
    endcase
  end
endmodule

