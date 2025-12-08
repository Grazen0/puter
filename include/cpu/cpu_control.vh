`ifndef PUTER_CPU_CONTROL_VH
`define PUTER_CPU_CONTROL_VH

`define ALU_SRC_B_RS2 1'd0
`define ALU_SRC_B_IMM 1'd1

`define RESULT_SRC_ALU 2'd0
`define RESULT_SRC_DATA 2'd1
`define RESULT_SRC_PC_PLUS_4 2'd2
`define RESULT_SRC_PC_TARGET 2'd3

`define JUMP_SRC_PC_TARGET 1'd0
`define JUMP_SRC_ALU 1'd1

`endif

