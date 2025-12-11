`default_nettype none `timescale 1ns / 1ps

module keyboard_controller (
    input wire rst_n,

    input wire ps2_data,
    input wire ps2_clk,

    output reg [7:0] data,
    output reg valid
);
  localparam S_IDLE = 2'd0;
  localparam S_READ_BYTE = 2'd1;
  localparam S_READ_PARITY = 2'd2;
  localparam S_READ_STOP = 2'd3;

  reg [1:0] state, state_next;

  reg [2:0] bit_ctr, bit_ctr_next;
  reg [7:0] data_next;

  reg valid_next;
  reg parity, parity_next;

  always @(*) begin
    state_next   = state;
    valid_next   = valid;
    data_next    = data;
    bit_ctr_next = bit_ctr;
    parity_next  = parity;

    case (state)
      S_IDLE: begin
        if (~ps2_data) begin
          state_next   = S_READ_BYTE;
          bit_ctr_next = 0;
          parity_next  = 0;
          valid_next   = 0;
        end
      end
      S_READ_BYTE: begin
        data_next    = {ps2_data, data[7:1]};
        parity_next  = parity ^ ps2_data;
        bit_ctr_next = bit_ctr + 1;

        if (bit_ctr_next == 0) begin
          state_next = S_READ_PARITY;
        end
      end
      S_READ_PARITY: begin
        valid_next = 1;
        state_next = parity == ps2_data ? S_READ_STOP : S_IDLE;
      end
      S_READ_STOP: begin
        if (ps2_data) begin
          state_next = S_IDLE;
          valid_next = 1;
        end else begin
          state_next   = S_READ_BYTE;
          bit_ctr_next = 0;
          parity_next  = 0;
          valid_next   = 0;
        end
      end
      default: state_next = S_IDLE;
    endcase
  end

  always @(negedge ps2_clk or negedge rst_n) begin
    if (!rst_n) begin
      state <= S_IDLE;
      valid <= 0;
    end else begin
      state   <= state_next;
      valid   <= valid_next;
      data    <= data_next;
      bit_ctr <= bit_ctr_next;
      parity  <= parity_next;
    end
  end
endmodule
