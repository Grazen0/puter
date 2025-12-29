`default_nettype none `timescale 1ns / 1ps

module spi_controller #(
    parameter CLK_FREQ = 50_000_000,
    parameter SCLK_FREQ = 25_000_000,
    parameter INIT_SCLK_FREQ = 400_000
) (
    input wire clk,
    input wire rst_n,

    // 00: set ss
    // 01: set clock half period
    // 10: write byte
    // 11: read byte
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
  localparam CMD_SET_SS = 2'b00;
  localparam CMD_SET_HALF_PERIOD = 2'b01;
  localparam CMD_WRITE = 2'b10;
  localparam CMD_READ = 2'b11;

  localparam S_IDLE = 2'd0;
  localparam S_WRITE = 2'd1;
  localparam S_WAIT_FOR_RESPONSE = 2'd2;
  localparam S_READ = 2'd3;

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

  wire sclk_posedge = ~sclk & sclk_next;
  wire sclk_negedge = sclk & ~sclk_next;

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
            CMD_SET_SS:          ss_next = data[0];
            CMD_SET_HALF_PERIOD: half_period_next = data;
            CMD_WRITE: begin
              state_next    = S_WRITE;
              data_buf_next = data;
              bit_ctr_next  = 0;
              mosi_next = data_buf_next[7];
            end
            CMD_READ: begin
              state_next       = S_READ;
              bit_ctr_next     = 0;
              rdata_valid_next = 0;
            end
            default: begin
            end
          endcase
        end
      end
      S_WRITE: begin
        ctr_next = ctr + 1;

        if (ctr_next >= half_period) begin
          sclk_next = ~sclk;
          ctr_next  = 0;
        end

        if (sclk_negedge) begin
          bit_ctr_next = bit_ctr - 1;
          state_next = bit_ctr_next == 0 ? S_IDLE : S_WRITE;
          data_buf_next = {data_buf[6:0], 1'b1};
          mosi_next = data_buf_next[7];
        end
      end
      S_READ: begin
        ctr_next = ctr + 1;

        if (ctr_next >= half_period) begin
          sclk_next = ~sclk;
          ctr_next  = 0;
        end

        if (sclk_posedge) begin
          bit_ctr_next  = bit_ctr - 1;
          data_buf_next = {data_buf[6:0], miso};
        end else if (sclk_negedge && bit_ctr_next == 0) begin
          state_next = S_IDLE;
          rdata_next = data_buf;
          rdata_valid_next = 1;
        end
      end
      default: state_next = S_IDLE;
    endcase
  end

  always @(posedge clk) begin
    if (!rst_n) begin
      sclk  <= 0;
      ss    <= 1;
      mosi  <= 1;
      state <= S_IDLE;
      ctr   <= 0;
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
