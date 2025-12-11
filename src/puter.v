`default_nettype none `timescale 1ns / 1ps

module puter (
    input wire sys_clk,
    input wire vga_clk,
    input wire rst_n,

    output wire [3:0] vga_red,
    output wire [3:0] vga_green,
    output wire [3:0] vga_blue,
    output wire h_sync,
    output wire v_sync
);
  localparam DATA_SEL_ROM = 2'd0;
  localparam DATA_SEL_RAM = 2'd1;
  localparam DATA_SEL_TRAM = 2'd2;

  wire [31:0] instr_addr;
  wire [31:0] instr_rdata;

  wire [31:0] data_addr;
  wire [31:0] data_wdata;
  wire [ 3:0] data_wenable;
  reg  [31:0] data_rdata;

  cpu cpu (
      .clk  (sys_clk),
      .rst_n(rst_n),

      .instr_addr(instr_addr),
      .instr_data(instr_rdata),

      .data_addr   (data_addr),
      .data_wdata  (data_wdata),
      .data_wenable(data_wenable),
      .data_rdata  (data_rdata)
  );

  reg [1:0] data_sel;

  always @(*) begin
    casez (data_addr[31:30])
      2'b0z:   data_sel = DATA_SEL_ROM;
      2'b10:   data_sel = DATA_SEL_RAM;
      2'b11:   data_sel = DATA_SEL_TRAM;
      default: data_sel = {32{1'bx}};
    endcase

    case (data_sel)
      DATA_SEL_ROM:  data_rdata = rom_rdata;
      DATA_SEL_RAM:  data_rdata = ram_rdata;
      DATA_SEL_TRAM: data_rdata = tram_rdata;
      default:       data_rdata = {32{1'bx}};
    endcase
  end

  wire [31:0] rom_rdata;

  dual_word_rom #(
      .SOURCE_FILE("/home/jdgt/Code/utec/arqui/puter/build/firmware/firmware.mem")
  ) rom (
      .addr_1 (instr_addr[14:0]),
      .rdata_1(instr_rdata),

      .addr_2 (data_addr[14:0]),
      .rdata_2(rom_rdata)
  );

  wire [31:0] ram_rdata;

  word_ram ram (
      .clk(sys_clk),

      .addr_1   (data_addr[14:0]),
      .wdata_1  (data_wdata),
      .wenable_1(data_wenable & {4{data_sel == DATA_SEL_RAM}}),
      .rdata_1  (ram_rdata)
  );

  wire [7:0] tram_rdata;

  video_unit video_unit (
      .sys_clk(sys_clk),
      .vga_clk(vga_clk),
      .rst_n  (rst_n),

      .tram_addr   (data_addr[11:0]),
      .tram_wdata  (data_wdata[7:0]),
      .tram_wenable(data_wenable[0] && data_sel == DATA_SEL_TRAM),
      .tram_rdata  (tram_rdata),

      .vga_red  (vga_red),
      .vga_green(vga_green),
      .vga_blue (vga_blue),
      .h_sync   (h_sync),
      .v_sync   (v_sync)
  );
endmodule
