`default_nettype none `timescale 1ns / 1ps
`include "spi_controller.vh"

module sd_card_dmac #(
    parameter XLEN = 32,
    parameter BLOCK_SIZE = 512
) (
    input wire clk,
    input wire rst_n,

    output wire            ready,
    input  wire            op,
    input  wire [XLEN-1:0] wdata,
    input  wire            start,

    input  wire       spi_ready,
    output wire [1:0] spi_cmd,
    output wire [7:0] spi_wdata,
    output reg        spi_start,
    input  wire [7:0] spi_rdata,
    input  wire       spi_rdata_valid,
    output wire       spi_req,

    output wire [XLEN-1:0] ram_addr,
    output wire [XLEN-1:0] ram_wdata,
    output wire [     3:0] ram_wenable,
    output wire            ram_req,

    output reg int_req
);
  localparam OP_START = 1'b0;
  localparam OP_SET_DEST = 1'b1;

  localparam S_IDLE = 1'd0;
  localparam S_ACTIVE = 1'd1;

  reg int_req_next;

  reg state, state_next;
  reg [XLEN-1:0] dest_start, dest_start_next;
  reg [XLEN-1:0] dest, dest_next;

  reg [$clog2(BLOCK_SIZE)-1:0] byte_ctr, byte_ctr_next;

  reg write_pending, write_pending_next;

  reg finished;

  always @(*) begin
    state_next         = state;
    dest_start_next    = dest_start;
    dest_next          = dest;
    byte_ctr_next      = byte_ctr;
    int_req_next       = int_req;
    write_pending_next = write_pending;

    spi_start          = 0;
    finished           = 0;

    case (state)
      S_IDLE: begin
        if (start) begin
          case (op)
            OP_START: begin
              state_next    = S_ACTIVE;
              dest_next     = dest_start;
              byte_ctr_next = 0;
              int_req_next  = 0;
            end
            OP_SET_DEST: begin
              dest_start_next = wdata;
            end
            default: begin
            end
          endcase
        end
      end
      S_ACTIVE: begin
        if (write_pending && spi_rdata_valid) begin
          write_pending_next = 0;
          dest_next          = dest + 1;
          byte_ctr_next      = byte_ctr + 1;

          if (byte_ctr_next == 0) begin
            finished = 1;
            state_next = S_IDLE;
            int_req_next = 1;
          end
        end

        if (spi_ready && !finished) begin
          spi_start = 1;
          write_pending_next = 1;
        end

      end
      default: state_next = S_IDLE;
    endcase
  end

  always @(posedge clk) begin
    if (!rst_n) begin
      state         <= S_IDLE;
      dest_start    <= {XLEN{1'bx}};
      dest          <= {XLEN{1'bx}};
      byte_ctr      <= {$clog2(BLOCK_SIZE) {1'bx}};
      int_req       <= 0;
      write_pending <= 0;
    end else begin
      state         <= state_next;
      dest_start    <= dest_start_next;
      dest          <= dest_next;
      byte_ctr      <= byte_ctr_next;
      int_req       <= int_req_next;
      write_pending <= write_pending_next;
    end
  end

  assign ready       = state == S_IDLE;

  assign spi_cmd     = `SPI_TRANSFER;
  assign spi_wdata   = 8'hFF;
  assign spi_req     = state == S_ACTIVE;

  assign ram_addr    = dest;
  assign ram_wdata   = spi_rdata;
  assign ram_wenable = {3'b000, spi_rdata_valid & write_pending};
  assign ram_req     = ram_wenable[0];
endmodule
