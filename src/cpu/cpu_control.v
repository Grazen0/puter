`default_nettype none `timescale 1ns / 1ps

`include "cpu_alu.vh"
`include "cpu_control.vh"
`include "cpu_csr_file.vh"
`include "cpu_imm_extend.vh"

module cpu_control (
    input wire [ 6:0] op,
    input wire [ 2:0] funct3,
    input wire [ 6:0] funct7,
    input wire [11:0] funct12,

    output reg       reg_write,
    output reg [2:0] result_src,
    output reg [3:0] mem_write,
    output reg       jump,
    output reg       branch,
    output reg [3:0] alu_control,
    output reg       alu_src_a,
    output reg [1:0] alu_src_b,
    output reg [2:0] imm_src,
    output reg [2:0] data_ext_control,
    output reg [1:0] jump_src,
    output reg [2:0] branch_cond,
    output reg       illegal_instr,
    output reg       csr_write,
    output reg       exception,
    output reg [1:0] exception_cause,
    output reg       mret
);
  always @(*) begin
    reg_write       = 0;
    result_src      = `RESULT_SRC_ALU;
    mem_write       = 4'b0000;
    jump            = 0;
    branch          = 0;
    alu_control     = 4'bxxxx;
    alu_src_a       = 1'bx;
    alu_src_b       = 2'bxx;
    imm_src         = 3'bxxx;
    jump_src        = 1'bx;
    branch_cond     = 3'bxxx;
    illegal_instr   = 0;
    csr_write       = 0;
    exception       = 0;
    exception_cause = 2'bxx;
    mret            = 0;

    case (op)
      7'b0000011: begin  // load
        imm_src          = `IMM_SRC_I;
        alu_src_a        = `ALU_SRC_A_RD1;
        alu_src_b        = `ALU_SRC_B_IMM;
        alu_control      = `ALU_ADD;

        data_ext_control = funct3;

        result_src       = `RESULT_SRC_DATA;
        reg_write        = 1;
      end
      7'b0010011: begin  // alu immediate
        imm_src     = `IMM_SRC_I;
        alu_src_a   = `ALU_SRC_A_RD1;
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
        alu_src_a   = `ALU_SRC_A_RD1;
        alu_src_b   = `ALU_SRC_B_IMM;
        alu_control = `ALU_ADD;

        case (funct3)
          3'b000: mem_write = 4'b0001;
          3'b001: mem_write = 4'b0011;
          3'b010: mem_write = 4'b1111;
          default: begin
            illegal_instr = 1;
          end
        endcase
      end
      7'b0110011: begin  // alu register
        imm_src     = `IMM_SRC_I;
        alu_src_a   = `ALU_SRC_A_RD1;
        alu_src_b   = `ALU_SRC_B_RD2;
        alu_control = {funct7[5], funct3};

        result_src  = `RESULT_SRC_ALU;
        reg_write   = 1;
      end
      7'b0110111: begin  // lui
        imm_src     = `IMM_SRC_U;
        alu_src_a   = `ALU_SRC_A_RD1;
        alu_src_b   = `ALU_SRC_B_IMM;
        alu_control = `ALU_PASS_B;
        result_src  = `RESULT_SRC_ALU;
        reg_write   = 1;
      end
      7'b1100011: begin  // branch
        imm_src     = `IMM_SRC_B;
        alu_src_a   = `ALU_SRC_A_RD1;
        alu_src_b   = `ALU_SRC_B_RD2;
        alu_control = `ALU_SUB;
        branch      = 1;
        branch_cond = funct3;
        jump_src    = `JUMP_SRC_PC_TARGET;
      end
      7'b1100111: begin  // jalr
        imm_src     = `IMM_SRC_I;
        alu_src_a   = `ALU_SRC_A_RD1;
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
      7'b1110011: begin  // privileged / csr instructions
        imm_src = `IMM_SRC_I;

        if (funct3 == 3'b000) begin
          case (funct12)
            12'h000: begin  // ecall
              exception = 1;
              exception_cause = `EXCAUSE_ECALL;
            end
            12'h001: begin  // ebreak
              exception = 1;
              exception_cause = `EXCAUSE_BREAKPOINT;
            end
            12'h002: begin  // uret
              $display("uret");
            end
            12'h102: begin  // sret
              $display("sret");
            end
            12'h302: begin  // mret
              jump = 1;
              jump_src = `JUMP_SRC_MEPC;
              mret = 1;
            end
            12'h105: begin  // wfi
              // Implemented as nop
            end
            default: begin
              illegal_instr = 1;
            end
          endcase
        end else begin
          alu_src_a = `ALU_SRC_A_CSR;
          alu_src_b = funct3[2] ? `ALU_SRC_B_RS1 : `ALU_SRC_B_RD1;

          case (funct3[1:0])
            2'b01: alu_control = `ALU_PASS_B;  // csrrw(i)
            2'b10: alu_control = `ALU_OR;  // csrrs(i)
            2'b11: alu_control = `ALU_AND_NOT;  // csrrc(i)
            default: begin
              illegal_instr = 1;
            end
          endcase

          result_src = `RESULT_SRC_CSR;

          reg_write  = 1;
          csr_write  = 1;
        end
      end
      default: begin
        illegal_instr = 1;
      end
    endcase

    if (illegal_instr) begin
      exception       = 1;
      exception_cause = `EXCAUSE_ILLEGAL_INSTR;

      reg_write       = 0;
      mem_write       = 4'b0000;
      jump            = 0;
      branch          = 0;
      illegal_instr   = 0;
      csr_write       = 0;
      exception       = 0;
    end
  end
endmodule

