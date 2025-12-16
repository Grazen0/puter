`default_nettype none `timescale 1ns / 1ps

module video_unit #(
    parameter FONT_DATA = "/home/jdgt/Code/utec/arqui/puter/build/unscii-16.mem"
) (
    input wire sys_clk,
    input wire vga_clk,
    input wire rst_n,

    input wire [$clog2(2 * TRAM_SIZE)-1:0] tram_addr,
    input wire [15:0] tram_wdata,
    input wire [1:0] tram_wenable,
    output wire [15:0] tram_rdata,

    output wire [3:0] vga_red,
    output wire [3:0] vga_green,
    output wire [3:0] vga_blue,
    output reg h_sync,
    output reg v_sync
);

  localparam WIDTH = 640;
  localparam HEIGHT = 480;

  localparam H_VISIBLE = 0;
  localparam H_FRONT = H_VISIBLE + WIDTH;
  localparam H_SYNC = H_FRONT + 16;
  localparam H_BACK = H_SYNC + 96;
  localparam H_LINE = H_BACK + 48;

  localparam V_VISIBLE = 0;
  localparam V_FRONT = V_VISIBLE + HEIGHT;
  localparam V_SYNC = V_FRONT + 10;
  localparam V_BACK = V_SYNC + 2;
  localparam V_FRAME = V_BACK + 33;

  localparam CHAR_WIDTH = 8;
  localparam CHAR_HEIGHT = 16;
  localparam CHAR_SIZE = CHAR_HEIGHT;
  localparam FONT_RAM_SIZE = 256 * CHAR_SIZE;

  localparam ROWS = HEIGHT / CHAR_HEIGHT;
  localparam COLS = WIDTH / CHAR_WIDTH;
  localparam TRAM_SIZE = ROWS * COLS;

  localparam PALETTE_SIZE = 16;

  reg [ 7:0] font_ram[0:FONT_RAM_SIZE-1];
  reg [11:0] palette [ 0:PALETTE_SIZE-1];

  initial begin
    palette[0]  = 12'h000;  // black
    palette[1]  = 12'h00A;  // blue
    palette[2]  = 12'h0A0;  // green
    palette[3]  = 12'h0AA;  // cyan
    palette[4]  = 12'hA00;  // red
    palette[5]  = 12'hA0A;  // magenta
    palette[6]  = 12'hA50;  // brown
    palette[7]  = 12'hAAA;  // gray

    palette[8]  = 12'h555;  // dark gray
    palette[9]  = 12'h55F;  // bright blue
    palette[10] = 12'h5F5;  // bright green
    palette[11] = 12'h5FF;  // bright cyan
    palette[12] = 12'hF55;  // bright red
    palette[13] = 12'hF5F;  // bright magenta
    palette[14] = 12'hFF5;  // yellow
    palette[15] = 12'hFFF;  // white
  end

  wire [15:0] active_entry;

  dual_half_word_ram #(
      .SIZE_HWORDS(TRAM_SIZE)
  ) tram (
      .clk(sys_clk),

      .addr_1   (tram_addr),
      .wdata_1  (tram_wdata),
      .wenable_1(tram_wenable),
      .rdata_1  (tram_rdata),

      .addr_2 ({char_idx, 1'b0}),
      .rdata_2(active_entry)
  );

  reg [$clog2(V_FRAME)-1:0] y_pos, y_pos_next;
  reg [$clog2(H_LINE)-1:0] x_pos, x_pos_next;
  reg h_visible, h_visible_next;
  reg v_visible, v_visible_next;
  reg h_sync_next;
  reg v_sync_next;
  reg [11:0] cur_color;

  always @(*) begin
    y_pos_next = y_pos;
    x_pos_next = x_pos + 1;

    h_visible_next = h_visible;
    v_visible_next = v_visible;

    if (x_pos_next == H_FRONT) begin
      h_visible_next = 0;
    end else if (x_pos_next == H_LINE) begin
      // Next line
      h_visible_next = 1;
      x_pos_next     = 0;
      y_pos_next     = y_pos + 1;

      if (y_pos_next == V_FRONT) begin
        v_visible_next = 0;
      end else if (y_pos_next == V_FRAME) begin
        // Next frame
        v_visible_next = 1;
        y_pos_next     = 0;
      end
    end

    case (x_pos_next)
      H_SYNC:  h_sync_next = 0;
      H_BACK:  h_sync_next = 1;
      default: h_sync_next = h_sync;
    endcase

    case (y_pos_next)
      V_SYNC:  v_sync_next = 0;
      V_BACK:  v_sync_next = 1;
      default: v_sync_next = v_sync;
    endcase
  end

  always @(posedge vga_clk) begin
    if (!rst_n) begin
      x_pos     <= 0;
      y_pos     <= 0;
      h_sync    <= 1;
      v_sync    <= 1;
      h_visible <= 1;
      v_visible <= 1;
      cur_color <= 12'h000;
    end else begin
      x_pos     <= x_pos_next;
      y_pos     <= y_pos_next;
      h_sync    <= h_sync_next;
      v_sync    <= v_sync_next;
      h_visible <= h_visible_next;
      v_visible <= v_visible_next;
      cur_color <= cur_color_next;
    end
  end

  wire [$clog2(COLS)-1:0] col = x_pos_next[3+:$clog2(COLS)];
  wire [$clog2(ROWS)-1:0] row = y_pos_next[4+:$clog2(ROWS)];

  wire [$clog2(TRAM_SIZE)-1:0] char_idx = (row * COLS) + col;

  wire [2:0] char_x = x_pos_next[2:0];
  wire [3:0] char_y = y_pos_next[3:0];

  wire [7:0] active_attr;
  wire [7:0] active_char;

  assign {active_attr, active_char} = active_entry;

  wire [3:0] fg_color_idx = active_attr[3:0];
  wire [3:0] bg_color_idx = active_attr[7:4];

  wire [11:0] fg_color = palette[fg_color_idx];
  wire [11:0] bg_color = palette[bg_color_idx];

  wire [$clog2(FONT_RAM_SIZE)-1:0] byte_idx = (CHAR_SIZE * active_char) + char_y;

  wire [7:0] cur_byte = font_ram[byte_idx];
  wire cur_pixel = cur_byte[7-char_x];

  wire [11:0] cur_color_next = cur_pixel ? fg_color : bg_color;

  wire visible = h_visible & v_visible;
  assign {vga_red, vga_green, vga_blue} = cur_color & {12{visible}};

  initial begin
    $readmemh(FONT_DATA, font_ram);
  end
endmodule
