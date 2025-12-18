`default_nettype none `timescale 1ns / 1ps

`include "cpu_alu.vh"

module cpu_alu #(
    parameter XLEN = 32
) (
    input wire [XLEN-1:0] src_a,
    input wire [XLEN-1:0] src_b,
    input wire [4:0] control,

    output reg  [XLEN-1:0] result,
    output reg             carry,
    output reg             overflow,
    output wire            zero,
    output wire            neg
);
  wire signed [XLEN-1:0] src_a_signed = src_a;
  wire signed [XLEN-1:0] src_b_signed = src_b;

  wire [4:0] shamt = src_b[4:0];

  wire [(2*XLEN)-1:0] mul_ss = src_a_signed * src_b_signed;
  wire [(2*XLEN)-1:0] mul_su = src_a_signed * src_b;
  wire [(2*XLEN)-1:0] mul_uu = src_a * src_b;

  always @(*) begin
    carry    = 0;
    overflow = 0;

    casez (control)
      `ALU_ADD: begin
        {carry, result} = {1'b0, src_a} + {1'b0, src_b};
        overflow = ~(src_a[XLEN-1] ^ src_b[XLEN-1]) & (src_a[XLEN-1] ^ result[XLEN-1]);
      end
      `ALU_SUB: begin
        {carry, result} = {1'b1, src_a} - {1'b0, src_b};
        overflow = (src_a[XLEN-1] ^ src_b[XLEN-1]) & (src_a[XLEN-1] ^ result[XLEN-1]);
      end
      `ALU_SLL:     result = src_a << shamt;
      `ALU_SLT:     result = {{(XLEN - 1) {1'b0}}, src_a_signed < src_b_signed};
      `ALU_SLTU:    result = {{(XLEN - 1) {1'b0}}, src_a < src_b};
      `ALU_XOR:     result = src_a ^ src_b;
      `ALU_SRL:     result = src_a >> shamt;
      `ALU_SRA:     result = src_a_signed >>> shamt;
      `ALU_OR:      result = src_a | src_b;
      `ALU_AND:     result = src_a & src_b;
      `ALU_PASS_A:  result = src_a;
      `ALU_PASS_B:  result = src_b;
      `ALU_AND_NOT: result = src_a & ~src_b;
      `ALU_MUL:     result = mul_uu[XLEN-1:0];
      `ALU_MULH:    result = mul_ss[(2*XLEN)-1:XLEN];
      `ALU_MULHSU:  result = mul_su[(2*XLEN)-1:XLEN];
      `ALU_MULHU:   result = mul_uu[(2*XLEN)-1:XLEN];
      `ALU_DIV:     result = src_a_signed / src_b_signed;
      `ALU_DIVU:    result = src_a / src_b;
      `ALU_REM:     result = src_a_signed % src_b_signed;
      `ALU_REMU:    result = src_a % src_b;
      default:      result = {XLEN{1'bx}};
    endcase
  end

  assign zero = result == 0;
  assign neg  = result[XLEN-1];
endmodule
