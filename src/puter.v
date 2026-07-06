`default_nettype none `timescale 1ns / 1ps

module puter #(
    parameter XLEN = 32
) (
    input wire sys_clk,
    input wire vga_clk,
    input wire rt_clk,
    input wire rst_n,

    output wire [3:0] vga_red,
    output wire [3:0] vga_green,
    output wire [3:0] vga_blue,
    output wire h_sync,
    output wire v_sync,

    input wire ps2_clk,
    input wire ps2_data,

    input  wire rs_rx,
    output wire rs_tx,

    output wire sd_sclk,
    output wire sd_cs,
    input  wire sd_miso,
    output wire sd_mosi
);
  localparam MEI_PORTS = 2;

  localparam INSTR_SEL_ROM = 1'b0;
  localparam INSTR_SEL_RAM = 1'b1;

  localparam DATA_SEL_ROM = 4'd0;
  localparam DATA_SEL_RAM = 4'd1;
  localparam DATA_SEL_TRAM = 4'd2;
  localparam DATA_SEL_RTC = 4'd3;
  localparam DATA_SEL_PLIC = 4'd4;
  localparam DATA_SEL_MEI_ID = 4'd5;
  localparam DATA_SEL_KEYBOARD_DATA = 4'd6;
  localparam DATA_SEL_UART = 4'd7;
  localparam DATA_SEL_VREGS = 4'd8;
  localparam DATA_SEL_SPI = 4'd9;
  localparam DATA_SEL_SD_DMAC = 4'd10;

  wire [XLEN-1:0] instr_addr;
  reg [XLEN-1:0] instr_rdata;

  wire [XLEN-1:0] data_addr;
  wire [XLEN-1:0] data_wdata;
  wire [3:0] data_wenable;
  reg [XLEN-1:0] data_rdata;

  cpu #(
      .MEI_PORTS(MEI_PORTS)
  ) cpu (
      .clk  (sys_clk),
      .rst_n(rst_n),
      .halt (sd_dmac_ram_req),

      .instr_addr(instr_addr),
      .instr_data(instr_rdata),

      .data_addr   (data_addr),
      .data_wdata  (data_wdata),
      .data_wenable(data_wenable),
      .data_rdata  (data_rdata),

      .mti_pending(mti_pending),
      .mei_pending(mei_pending)
  );

  reg       instr_sel;
  reg [3:0] data_sel;

  always @(*) begin
    casez (instr_addr[XLEN-1])
      1'b0:    instr_sel = INSTR_SEL_ROM;
      1'b1:    instr_sel = INSTR_SEL_RAM;
      default: instr_sel = 1'bx;
    endcase

    casez (data_addr[XLEN-1:XLEN-5])
      5'b0000_0: data_sel = DATA_SEL_ROM;
      5'b0000_1: data_sel = DATA_SEL_TRAM;
      5'b0001_0: data_sel = DATA_SEL_VREGS;
      5'b0001_1: data_sel = DATA_SEL_UART;
      5'b0010_0: data_sel = DATA_SEL_SPI;
      5'b0010_1: data_sel = DATA_SEL_RTC;
      5'b0011_0: data_sel = DATA_SEL_KEYBOARD_DATA;
      5'b0011_1: data_sel = DATA_SEL_PLIC;
      5'b010z_z: data_sel = DATA_SEL_MEI_ID;
      5'b011z_z: data_sel = DATA_SEL_SD_DMAC;
      5'b1zzz_z: data_sel = DATA_SEL_RAM;
      default:   data_sel = 4'bxxxx;
    endcase

    case (instr_sel)
      INSTR_SEL_ROM: instr_rdata = rom_rinstr;
      INSTR_SEL_RAM: instr_rdata = ram_rinstr;
      default:       instr_rdata = {32{1'bx}};
    endcase

    case (data_sel)
      DATA_SEL_ROM:           data_rdata = rom_rdata;
      DATA_SEL_RAM:           data_rdata = ram_rdata;
      DATA_SEL_TRAM:          data_rdata = tram_rdata;
      DATA_SEL_VREGS:         data_rdata = vregs_rdata;
      DATA_SEL_UART:          data_rdata = uart_ready;
      DATA_SEL_SPI: begin
        casez (data_addr[1:0])
          2'b00:   data_rdata = spi_ready;
          2'b01:   data_rdata = spi_rdata_valid;
          2'b1z:   data_rdata = spi_rdata;
          default: data_rdata = {32{1'bx}};
        endcase
      end
      DATA_SEL_RTC:           data_rdata = rtc_rdata;
      DATA_SEL_PLIC:          data_rdata = plic_rdata;
      DATA_SEL_MEI_ID:        data_rdata = mei_id;
      DATA_SEL_KEYBOARD_DATA: data_rdata = keyboard_data;
      DATA_SEL_SD_DMAC:       data_rdata = sd_dmac_ready;
      default:                data_rdata = {32{1'bx}};
    endcase
  end

  wire [XLEN-1:0] rom_rinstr;
  wire [XLEN-1:0] rom_rdata;

  dual_word_rom #(
      .SIZE_BYTES(8 * (2 ** 10)),  // 8K
      .SOURCE_FILE("/home/jdgt/Code/verilog/puter/firmware/build/firmware.mem")
  ) rom (
      .addr_1 (instr_addr[12:0]),
      .rdata_1(rom_rinstr),

      .addr_2 (data_addr[12:0]),
      .rdata_2(rom_rdata)
  );

  wire [XLEN-1:0] ram_rdata;
  wire [XLEN-1:0] ram_rinstr;

  dual_word_ram #(
      .SIZE_BYTES(32 * (2 ** 10))  // 32K
  ) ram (
      .clk(sys_clk),

      .addr_1(sd_dmac_ram_req ? sd_dmac_ram_addr[14:0] : data_addr[14:0]),
      .wdata_1(sd_dmac_ram_req ? sd_dmac_ram_wdata : data_wdata),
      .wenable_1(sd_dmac_ram_req ? sd_dmac_ram_wenable : data_wenable & {4{data_sel == DATA_SEL_RAM}}),
      .rdata_1(ram_rdata),

      .addr_2 (instr_addr[14:0]),
      .rdata_2(ram_rinstr)
  );

  wire [15:0] tram_rdata;
  wire [15:0] vregs_rdata;

  video_unit #(
      .FONT_DATA("/home/jdgt/Code/verilog/puter/build/unscii-16.mem")
  ) video_unit (
      .sys_clk(sys_clk),
      .vga_clk(vga_clk),
      .rst_n  (rst_n),

      .tram_addr   (data_addr[12:0]),
      .tram_wdata  (data_wdata[15:0]),
      .tram_wenable(data_wenable[1:0] & {2{data_sel == DATA_SEL_TRAM}}),
      .tram_rdata  (tram_rdata),

      .reg_sel    (data_addr[2:1]),
      .reg_wdata  (data_wdata[15:0]),
      .reg_wenable(|data_wenable[1:0] & data_sel == DATA_SEL_VREGS),
      .reg_rdata  (vregs_rdata),

      .vga_red  (vga_red),
      .vga_green(vga_green),
      .vga_blue (vga_blue),
      .h_sync   (h_sync),
      .v_sync   (v_sync)
  );

  wire [XLEN-1:0] rtc_rdata;
  wire mti_pending;

  rt_counter rt_counter (
      .clk(rt_clk),
      .sys_clk(sys_clk),
      .rst_n(rst_n),

      .reg_sel(data_addr[3]),
      .h_sel  (data_addr[2]),
      .wdata  (data_wdata),
      .wenable(|data_wenable && data_sel == DATA_SEL_RTC),
      .rdata  (rtc_rdata),

      .int_pending(mti_pending)
  );

  localparam PLIC_PRIORITY_WIDTH = $clog2(MEI_PORTS + 1);

  wire mei_pending;
  wire mei_id;
  wire [PLIC_PRIORITY_WIDTH-1:0] plic_rdata;

  plic #(
      .PORTS(MEI_PORTS)
  ) plic (
      .clk  (sys_clk),
      .rst_n(rst_n),

      .port_sel(data_addr[0]),
      .action_sel(data_addr[2:1]),
      .wdata(data_wdata[PLIC_PRIORITY_WIDTH-1:0]),
      .wenable(|data_wenable && data_sel == DATA_SEL_PLIC),
      .rdata(plic_rdata),

      .int_signal({sd_dmac_int_req, keyboard_valid}),

      .out_int_pending(mei_pending),
      .out_int_id     (mei_id)
  );

  wire [7:0] keyboard_data;
  wire keyboard_valid;

  keyboard_controller keyboard_controller (
      .rst_n(rst_n),

      .ps2_clk (ps2_clk),
      .ps2_data(ps2_data),

      .data (keyboard_data),
      .valid(keyboard_valid)
  );

  wire uart_ready;

  uart_controller uart (
      .clk  (sys_clk),
      .rst_n(rst_n),

      .write_data (data_wdata[7:0]),
      .start_write(data_wenable[0] && data_sel == DATA_SEL_UART),
      .ready      (uart_ready),

      .rx(rs_rx),
      .tx(rs_tx)
  );

  wire       spi_ready;
  wire [7:0] spi_rdata;
  wire       spi_rdata_valid;

  spi_controller spi (
      .clk  (sys_clk),
      .rst_n(rst_n),

      .cmd(sd_dmac_spi_req ? sd_dmac_spi_cmd : data_addr[1:0]),
      .data(sd_dmac_spi_req ? sd_dmac_spi_wdata : data_wdata[7:0]),
      .start(sd_dmac_spi_req ? sd_dmac_spi_start : (data_wenable[0] && data_sel == DATA_SEL_SPI)),
      .ready(spi_ready),
      .rdata_valid(spi_rdata_valid),
      .rdata(spi_rdata),

      .sclk(sd_sclk),
      .ss  (sd_cs),
      .miso(sd_miso),
      .mosi(sd_mosi)
  );

  wire            sd_dmac_ready;

  wire [     1:0] sd_dmac_spi_cmd;
  wire [     7:0] sd_dmac_spi_wdata;
  wire            sd_dmac_spi_start;
  wire            sd_dmac_spi_req;

  wire [XLEN-1:0] sd_dmac_ram_addr;
  wire [XLEN-1:0] sd_dmac_ram_wdata;
  wire [     3:0] sd_dmac_ram_wenable;
  wire            sd_dmac_ram_req;

  wire            sd_dmac_int_req;

  sd_card_dmac sd_dmac (
      .clk  (sys_clk),
      .rst_n(rst_n),

      .ready(sd_dmac_ready),
      .op   (data_addr[2]),
      .wdata(data_wdata),
      .start(data_wenable[0] & data_sel == DATA_SEL_SD_DMAC),

      .spi_ready      (spi_ready),
      .spi_cmd        (sd_dmac_spi_cmd),
      .spi_wdata      (sd_dmac_spi_wdata),
      .spi_start      (sd_dmac_spi_start),
      .spi_rdata      (spi_rdata),
      .spi_rdata_valid(spi_rdata_valid),
      .spi_req        (sd_dmac_spi_req),

      .ram_addr   (sd_dmac_ram_addr),
      .ram_wdata  (sd_dmac_ram_wdata),
      .ram_wenable(sd_dmac_ram_wenable),
      .ram_req    (sd_dmac_ram_req),

      .int_req(sd_dmac_int_req)
  );
endmodule
