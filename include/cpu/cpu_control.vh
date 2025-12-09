`ifndef PUTER_CPU_CONTROL_VH
`define PUTER_CPU_CONTROL_VH

`define ALU_SRC_A_RD1 1'd0
`define ALU_SRC_A_CSR 1'd1

`define ALU_SRC_B_RD2 2'd0
`define ALU_SRC_B_IMM 2'd1
`define ALU_SRC_B_RD1 2'd2
`define ALU_SRC_B_RS1 2'd3

`define RESULT_SRC_ALU 3'd0
`define RESULT_SRC_DATA 3'd1
`define RESULT_SRC_PC_PLUS_4 3'd2
`define RESULT_SRC_PC_TARGET 3'd3
`define RESULT_SRC_CSR 3'd4

`define JUMP_SRC_PC_TARGET 1'd0
`define JUMP_SRC_ALU 1'd1

`endif

