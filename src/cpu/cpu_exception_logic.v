`default_nettype none `timescale 1ns / 1ps

module cpu_exception_logic (
    input wire [1:0] priv,
    input wire [1:0] required_priv,

    input wire csr_write,
    input wire [11:0] csrs,

    input wire       control_exception,
    input wire [1:0] control_excause,

    output reg exception,
    output reg [1:0] excause
);

  wire [1:0] csr_priv = csrs[9:8];

  always @(*) begin
    exception = 0;
    excause   = 2'bxx;

    if (control_exception) begin
      exception = 1;
      excause   = control_excause;
    end else if (priv < required_priv || (csr_write && priv < csr_priv)) begin
      exception = 1;
      excause   = `EXCAUSE_ILLEGAL_INSTR;
    end
  end
endmodule
