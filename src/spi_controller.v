`default_nettype none `timescale 1ns / 1ps
`include "spi_controller.vh"

module spi_controller #(
    parameter CLK_FREQ = 50_000_000,
    parameter SCLK_FREQ = 25_000_000,
    parameter INIT_SCLK_FREQ = 400_000
) (
    input wire clk,
    input wire rst_n,

    // 00: set ss
    // 01: set clock half period
    // 1x: transfer byte
    input wire [1:0] cmd,
    input wire [7:0] data,
    input wire start,
    output wire ready,

    output reg [7:0] rdata,
    output reg rdata_valid,

    output reg  sclk,
    output reg  ss,
    input  wire miso,
    output reg  mosi
);
  localparam S_IDLE = 2'd0;
  localparam S_CLK_DOWN = 2'd1;
  localparam S_CLK_UP = 2'd2;

  reg sclk_next;
  reg ss_next;
  reg mosi_next;
  reg [7:0] rdata_next;
  reg rdata_valid_next;

  reg [1:0] state, state_next;

  reg [7:0] ctr, ctr_next;

  reg [7:0] data_buf, data_buf_next;
  reg [2:0] bit_ctr, bit_ctr_next;

  reg [7:0] half_period, half_period_next;

  always @(*) begin
    sclk_next        = sclk;
    ss_next          = ss;
    mosi_next        = mosi;
    state_next       = state;
    ctr_next         = ctr;
    data_buf_next    = data_buf;
    bit_ctr_next     = bit_ctr;
    half_period_next = half_period;
    rdata_next       = rdata;
    rdata_valid_next = rdata_valid;

    case (state)
      S_IDLE: begin
        sclk_next = 0;

        if (start) begin
          case (cmd)
            `SPI_SET_SS:          ss_next = data[0];
            `SPI_SET_HALF_PERIOD: half_period_next = data;
            `SPI_TRANSFER: begin
              state_next    = S_CLK_DOWN;
              data_buf_next = data;
              bit_ctr_next  = 0;
              mosi_next = data_buf_next[7];
              rdata_valid_next = 0;
            end
            default: begin
            end
          endcase
        end
      end
      S_CLK_DOWN: begin
        ctr_next = ctr + 1;

        if (ctr_next >= half_period) begin
          data_buf_next = {data_buf[6:0], miso};
          state_next    = S_CLK_UP;
          ctr_next      = 0;
        end
      end
      S_CLK_UP: begin
        ctr_next = ctr + 1;

        if (ctr_next >= half_period) begin
          bit_ctr_next = bit_ctr - 1;
          ctr_next     = 0;

          if (bit_ctr_next == 0) begin
            state_next       = S_IDLE;
            rdata_next       = data_buf;
            rdata_valid_next = 1;
          end else begin
            state_next = S_CLK_DOWN;
          end
        end
      end
      default: state_next = S_IDLE;
    endcase

    case (state_next)
      S_IDLE: begin
        sclk_next = 0;
      end
      S_CLK_DOWN: begin
        sclk_next = 0;
        mosi_next = data_buf_next[7];
      end
      S_CLK_UP: begin

        sclk_next = 1;
      end
      default: begin
      end
    endcase
  end

  always @(posedge clk) begin
    if (!rst_n) begin
      sclk        <= 0;
      ss          <= 1;
      mosi        <= 1;
      state       <= S_IDLE;
      ctr         <= 0;
      rdata       <= 0;
      rdata_valid <= 0;
    end else begin
      sclk        <= sclk_next;
      ss          <= ss_next;
      mosi        <= mosi_next;
      state       <= state_next;
      ctr         <= ctr_next;
      data_buf    <= data_buf_next;
      bit_ctr     <= bit_ctr_next;
      half_period <= half_period_next;
      rdata       <= rdata_next;
      rdata_valid <= rdata_valid_next;
    end
  end

  assign ready = state == S_IDLE;
endmodule
