`ifndef PUTER_CPU_CSR_FILE_VH
`define PUTER_CPU_CSR_FILE_VH

// TODO: implement mcounteren

`define CSR_SSTATUS 12'h100
`define CSR_SSTATUSH 12'h110
`define CSR_SIE 12'h104
`define CSR_STVEC 12'h105

`define CSR_SSCRATCH 12'h140
`define CSR_SEPC 12'h141
`define CSR_SCAUSE 12'h142
`define CSR_SIP 12'h144

`define CSR_MSTATUS 12'h300
`define CSR_MIE 12'h304
`define CSR_MTVEC 12'h305
`define CSR_MCOUNTEREN 12'h306
`define CSR_MSTATUSH 12'h310

`define CSR_MSCRATCH 12'h340
`define CSR_MEPC 12'h341
`define CSR_MCAUSE 12'h342
`define CSR_MIP 12'h344

`define CSR_MCYCLE 12'hB00
`define CSR_MCYCLEH 12'hB80
`define CSR_MINSTRET 12'hB02
`define CSR_MINSTRETH 12'hB82

`define PRIV_U 2'd0
`define PRIV_S 2'd1
`define PRIV_M 2'd3

`define MSTATUS_SIE 1
`define MSTATUS_MIE 3
`define MSTATUS_SPIE 5
`define MSTATUS_MPIE 7
`define MSTATUS_SPP 8
`define MSTATUS_MPP 12:11
`define MSTATUS_XS 16:15
`define MSTATUS_MPRV 17
`define MSTATUS_UXL 33:32
`define MSTATUS_SXL 35:34
`define MSTATUS_SBE 36
`define MSTATUS_MBE 37
`define MSTATUS_GVA 38


`endif
