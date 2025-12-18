`ifndef PUTER_CPU_ALU_VH
`define PUTER_CPU_ALU_VH

`define ALU_ADD 5'b00000
`define ALU_SUB 5'b01000
`define ALU_SLL 5'b00001
`define ALU_SLT 5'b00010
`define ALU_SLTU 5'b00011
`define ALU_XOR 5'b00100
`define ALU_SRL 5'b00101
`define ALU_SRA 5'b01101
`define ALU_OR 5'b00110
`define ALU_AND 5'b00111

`define ALU_PASS_A 5'b01001
`define ALU_PASS_B 5'b01010
`define ALU_AND_NOT 5'b01011

`define ALU_MUL 5'b10000
`define ALU_MULH 5'b10001
`define ALU_MULHSU 5'b10010
`define ALU_MULHU 5'b10011
`define ALU_DIV 5'b10100
`define ALU_DIVU 5'b10101
`define ALU_REM 5'b10110
`define ALU_REMU 5'b10111

`endif
