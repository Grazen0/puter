`default_nettype none `timescale 1ns / 1ps

module uart_controller #(
    parameter CLK_FREQ  = 50_000_000,
    parameter BAUD_RATE = 9600
) (
    input wire clk,
    input wire rst_n,

    input wire [7:0] write_data,
    input wire       start_write,

    output wire ready,

    input  wire rx,
    output reg  tx
);
  localparam BIT_DELAY = CLK_FREQ / BAUD_RATE;
  // localparam BIT_DELAY = 1;
  localparam CTR_WIDTH = $clog2(BIT_DELAY + 1);

  localparam S_IDLE = 3'd0;
  localparam S_START = 3'd1;
  localparam S_WRITE_DATA = 3'd2;
  localparam S_WRITE_PARITY = 3'd3;
  localparam S_STOP = 3'd4;

  reg [CTR_WIDTH-1:0] state_delay;

  reg tx_next;
  reg [7:0] data, data_next;
  reg parity, parity_next;
  reg [CTR_WIDTH-1:0] ctr, ctr_next;
  reg [2:0] bit_ctr, bit_ctr_next;
  reg [2:0] state, state_next_real, state_next;

  wire transition_pending = ctr + 1 >= state_delay;

  always @(*) begin
    tx_next         = tx;
    data_next       = data;
    state_next_real = state;
    state_next      = state;
    bit_ctr_next    = bit_ctr;
    parity_next     = parity;

    state_delay     = 0;

    case (state)
      S_IDLE: begin
        if (start_write) begin
          state_next  = S_START;
          data_next   = write_data;
          parity_next = 0;
        end
      end
      S_START: begin
        state_next   = S_WRITE_DATA;
        state_delay  = BIT_DELAY;

        bit_ctr_next = 0;
      end
      S_WRITE_DATA: begin
        state_delay = BIT_DELAY;

        if (transition_pending) begin
          data_next    = data >> 1;
          bit_ctr_next = bit_ctr + 1;

          if (bit_ctr_next == 0) begin
            state_next = S_WRITE_PARITY;
          end
        end
      end
      S_WRITE_PARITY: begin
        state_next  = S_STOP;
        state_delay = BIT_DELAY;
      end
      S_STOP: begin
        state_next  = S_IDLE;
        state_delay = BIT_DELAY;
      end
      default: begin
        state_next = S_IDLE;
      end
    endcase

    ctr_next = ctr + 1;

    if (transition_pending) begin
      ctr_next = 0;
      state_next_real = state_next;

      case (state_next_real)
        S_IDLE:         tx_next = 1;
        S_START:        tx_next = 0;
        S_WRITE_DATA: begin
          tx_next     = data_next[0];
          parity_next = parity ^ tx_next;
        end
        S_WRITE_PARITY: tx_next = parity;
        S_STOP:         tx_next = 1;
        default:        tx_next = 1;
      endcase
    end

  end

  always @(posedge clk) begin
    if (!rst_n) begin
      tx    <= 1;
      state <= S_IDLE;
      ctr   <= 0;
    end else begin
      tx      <= tx_next;
      data    <= data_next;
      ctr     <= ctr_next;
      state   <= state_next_real;
      bit_ctr <= bit_ctr_next;
      parity  <= parity_next;
    end
  end

  assign ready = state == S_IDLE;
endmodule
