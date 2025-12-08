`default_nettype none

`include "cpu_control.vh"
`include "cpu_hazard_unit.vh"

module cpu (
    input wire clk,
    input wire rst_n,

    output wire [31:0] instr_addr,
    input  wire [31:0] instr_data,

    output wire [31:0] data_addr,
    output wire [31:0] data_wdata,
    output wire [ 3:0] data_wenable,
    input  wire [31:0] data_rdata
);

  wire [1:0] forward_a_e;
  wire [1:0] forward_b_e;
  wire stall_f;
  wire stall_d;
  wire flush_d;
  wire flush_e;

  cpu_hazard_unit hazard_unit (
      .rs1_d(rs1_d),
      .rs2_d(rs2_d),

      .rs1_e(rs1_e),

      .rs2_e(rs2_e),
      .rd_e(rd_e),
      .pc_src_e(pc_src_e),
      .result_src_e(result_src_e),

      .reg_write_m(reg_write_m),
      .rd_m(rd_m),

      .reg_write_w(reg_write_w),
      .rd_w(rd_w),

      .forward_a_e(forward_a_e),
      .forward_b_e(forward_b_e),

      .stall_f(stall_f),
      .stall_d(stall_d),

      .flush_d(flush_d),
      .flush_e
  );

  // 1. Fetch
  reg [31:0] pc_next_f;

  always @(*) begin
    case (pc_src_e)
      `PC_SRC_PC_PLUS_4: pc_next_f = pc_plus_4_f;
      `PC_SRC_PC_TARGET: pc_next_f = pc_target_e;
      `PC_SRC_ALU:       pc_next_f = alu_result_e;
      default:           pc_next_f = {32{1'bx}};
    endcase
  end

  reg [31:0] pc_f;

  always @(posedge clk) begin
    if (!rst_n) begin
      pc_f <= 0;
    end else if (!stall_f) begin
      pc_f <= pc_next_f;
    end
  end

  assign instr_addr = pc_f;
  wire [31:0] instr_f = instr_data;

  wire [31:0] pc_plus_4_f = pc_f + 4;

  // 2. Decode
  reg  [31:0] instr_d;
  reg  [31:0] pc_d;
  reg  [31:0] pc_plus_4_d;

  always @(posedge clk) begin
    if (!rst_n || flush_d) begin
      instr_d     <= 32'h0000_0013;  // nop
      pc_d        <= {32{1'bx}};
      pc_plus_4_d <= {32{1'bx}};
    end else if (!stall_d) begin
      instr_d     <= instr_f;
      pc_d        <= pc_f;
      pc_plus_4_d <= pc_plus_4_f;
    end
  end

  wire [6:0] op_d = instr_d[6:0];
  wire [2:0] funct3_d = instr_d[14:12];
  wire [6:0] funct7_d = instr_d[31:25];

  wire       reg_write_d;
  wire [1:0] result_src_d;
  wire [3:0] mem_write_d;
  wire       jump_d;
  wire       branch_d;
  wire [3:0] alu_control_d;
  wire       alu_src_b_d;
  wire [2:0] imm_src_d;
  wire [2:0] data_ext_control_d;
  wire       jump_src_d;
  wire [2:0] branch_cond_d;
  wire       illegal_instr_d;

  cpu_control control (
      .op(op_d),
      .funct3(funct3_d),
      .funct7(funct7_d),

      .reg_write(reg_write_d),
      .result_src(result_src_d),
      .mem_write(mem_write_d),
      .jump(jump_d),
      .branch(branch_d),
      .alu_control(alu_control_d),
      .alu_src_b(alu_src_b_d),
      .imm_src(imm_src_d),
      .data_ext_control(data_ext_control_d),
      .jump_src(jump_src_d),
      .branch_cond(branch_cond_d),
      .illegal_instr(illegal_instr_d)
  );

  wire [31:0] rd1_d;
  wire [31:0] rd2_d;


  wire [ 4:0] rs1_d = instr_d[19:15];
  wire [ 4:0] rs2_d = instr_d[24:20];
  wire [ 4:0] rd_d = instr_d[11:7];

  cpu_register_file reg_file (
      .clk(~clk),

      .a1(rs1_d),
      .a2(rs2_d),

      .rd1(rd1_d),
      .rd2(rd2_d),

      .a3 (rd_w),
      .wd3(result_w),
      .we3(reg_write_w)
  );

  wire [31:0] imm_ext_d;

  cpu_imm_extend imm_ext (
      .data(instr_d[31:7]),
      .imm_src(imm_src_d),
      .imm_ext(imm_ext_d)
  );

  // 3. Execute
  reg        reg_write_e;
  reg [ 1:0] result_src_e;
  reg [ 3:0] mem_write_e;
  reg        jump_e;
  reg        branch_e;
  reg [ 3:0] alu_control_e;
  reg        alu_src_b_e;
  reg [ 2:0] data_ext_control_e;
  reg        jump_src_e;
  reg [ 2:0] branch_cond_e;
  reg        illegal_instr_e;

  reg [31:0] rd1_e;
  reg [31:0] rd2_e;
  reg [31:0] pc_e;
  reg [ 4:0] rs1_e;
  reg [ 4:0] rs2_e;
  reg [ 4:0] rd_e;
  reg [31:0] imm_ext_e;
  reg [31:0] pc_plus_4_e;

  always @(posedge clk) begin
    if (!rst_n || flush_e) begin
      reg_write_e        <= 0;
      result_src_e       <= `RESULT_SRC_ALU;
      mem_write_e        <= 4'b0000;
      jump_e             <= 0;
      branch_e           <= 0;
      alu_control_e      <= 4'bxxxx;
      alu_src_b_e        <= 1'bx;
      data_ext_control_e <= 3'bxxx;
      jump_src_e         <= 0;
      branch_cond_e      <= 3'bxxx;
      illegal_instr_e    <= 0;

      rd1_e              <= {32{1'bx}};
      rd2_e              <= {32{1'bx}};
      pc_e               <= {32{1'bx}};
      rs1_e              <= 5'bxxxxx;
      rs2_e              <= 5'bxxxxx;
      rd_e               <= 5'bxxxxx;
      imm_ext_e          <= {32{1'bx}};
      pc_plus_4_e        <= {32{1'bx}};
    end else begin
      reg_write_e        <= reg_write_d;
      result_src_e       <= result_src_d;
      mem_write_e        <= mem_write_d;
      jump_e             <= jump_d;
      branch_e           <= branch_d;
      alu_control_e      <= alu_control_d;
      alu_src_b_e        <= alu_src_b_d;
      data_ext_control_e <= data_ext_control_d;
      jump_src_e         <= jump_src_d;
      branch_cond_e      <= branch_cond_d;
      illegal_instr_e    <= illegal_instr_d;

      rd1_e              <= rd1_d;
      rd2_e              <= rd2_d;
      pc_e               <= pc_d;
      rs1_e              <= rs1_d;
      rs2_e              <= rs2_d;
      rd_e               <= rd_d;
      imm_ext_e          <= imm_ext_d;
      pc_plus_4_e        <= pc_plus_4_d;
    end
  end

  always @(posedge clk) begin
    #1;
    if (illegal_instr_e) begin
      $display("[illegal instruction at pc = %h]", pc_e);
    end
  end

  reg [31:0] rd1_fw_e;
  reg [31:0] rd2_fw_e;

  always @(*) begin
    case (forward_a_e)
      `FORWARD_NONE:      rd1_fw_e = rd1_e;
      `FORWARD_WRITEBACK: rd1_fw_e = result_w;
      `FORWARD_MEMORY:    rd1_fw_e = result_pre_m;
      default:            rd1_fw_e = {32{1'bx}};
    endcase

    case (forward_b_e)
      `FORWARD_NONE:      rd2_fw_e = rd2_e;
      `FORWARD_WRITEBACK: rd2_fw_e = result_w;
      `FORWARD_MEMORY:    rd2_fw_e = result_pre_m;
      default:            rd2_fw_e = {32{1'bx}};
    endcase
  end

  wire [31:0] alu_src_a_val_e = rd1_fw_e;
  wire [31:0] alu_src_b_val_e = alu_src_b_e == `ALU_SRC_B_RS2 ? rd2_fw_e : imm_ext_e;

  wire [31:0] alu_result_e;
  wire alu_carry_e;
  wire alu_overflow_e;
  wire alu_zero_e;
  wire alu_neg_e;

  cpu_alu alu (
      .src_a  (alu_src_a_val_e),
      .src_b  (alu_src_b_val_e),
      .control(alu_control_e),

      .result(alu_result_e),
      .carry(alu_carry_e),
      .overflow(alu_overflow_e),
      .zero(alu_zero_e),
      .neg(alu_neg_e)
  );

  wire [31:0] pc_target_e = pc_e + imm_ext_e;

  wire [ 1:0] pc_src_e;

  cpu_branch_logic branch_logic (
      .jump(jump_e),
      .jump_src(jump_src_e),
      .branch(branch_e),
      .branch_cond(branch_cond_e),

      .alu_carry(alu_carry_e),
      .alu_overflow(alu_overflow_e),
      .alu_zero(alu_zero_e),
      .alu_neg(alu_neg_e),

      .pc_src(pc_src_e)
  );

  wire [31:0] write_data_e = rd2_fw_e;

  // 4. Memory
  reg         reg_write_m;
  reg  [ 1:0] result_src_m;
  reg  [ 3:0] mem_write_m;
  reg  [ 2:0] data_ext_control_m;

  reg  [31:0] alu_result_m;
  reg  [31:0] write_data_m;
  reg  [ 4:0] rd_m;
  reg  [31:0] pc_plus_4_m;
  reg  [31:0] pc_target_m;

  always @(posedge clk) begin
    if (!rst_n) begin
      reg_write_m        <= 0;
      result_src_m       <= 2'bxx;
      mem_write_m        <= 4'b0000;
      data_ext_control_m <= 3'bxxx;

      alu_result_m       <= {32{1'bx}};
      write_data_m       <= {32{1'bx}};
      rd_m               <= 5'bxxxxx;
      pc_plus_4_m        <= {32{1'bx}};
      pc_target_m        <= {32{1'bx}};
    end else begin
      reg_write_m        <= reg_write_e;
      result_src_m       <= result_src_e;
      mem_write_m        <= mem_write_e;
      data_ext_control_m <= data_ext_control_e;

      alu_result_m       <= alu_result_e;
      write_data_m       <= write_data_e;
      rd_m               <= rd_e;
      pc_plus_4_m        <= pc_plus_4_e;
      pc_target_m        <= pc_target_e;
    end
  end

  assign data_addr = alu_result_m;
  assign data_wdata = write_data_m;
  assign data_wenable = mem_write_m;

  wire [31:0] read_data_m;

  cpu_data_extend data_extend (
      .data(data_rdata),
      .control(data_ext_control_m),
      .data_ext(read_data_m)
  );

  reg [31:0] result_pre_m;

  always @(*) begin
    case (result_src_m)
      `RESULT_SRC_ALU:       result_pre_m = alu_result_m;
      `RESULT_SRC_PC_PLUS_4: result_pre_m = pc_plus_4_m;
      `RESULT_SRC_PC_TARGET: result_pre_m = pc_target_m;
      default:               result_pre_m = {32{1'bx}};
    endcase
  end

  // 5. Writeback
  reg        reg_write_w;
  reg [ 1:0] result_src_w;

  reg [31:0] result_pre_w;
  reg [31:0] read_data_w;
  reg [ 4:0] rd_w;
  reg [31:0] pc_plus_4_w;
  reg [31:0] pc_target_w;

  always @(posedge clk) begin
    if (!rst_n) begin
      reg_write_w  <= 0;
      result_src_w <= 2'bxx;

      result_pre_w <= {32{1'bx}};
      read_data_w  <= {32{1'bx}};
      rd_w         <= 5'bxxxxx;
      pc_plus_4_w  <= {32{1'bx}};
      pc_target_w  <= {32{1'bx}};
    end else begin
      reg_write_w  <= reg_write_m;
      result_src_w <= result_src_m;

      result_pre_w <= result_pre_m;
      read_data_w  <= read_data_m;
      rd_w         <= rd_m;
      pc_plus_4_w  <= pc_plus_4_m;
      pc_target_w  <= pc_target_m;
    end
  end

  wire [31:0] result_w = result_src_w == `RESULT_SRC_DATA ? read_data_w : result_pre_w;
endmodule
