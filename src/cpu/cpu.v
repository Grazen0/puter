`default_nettype none `timescale 1ns / 1ps

`include "cpu_branch_logic.vh"
`include "cpu_control.vh"
`include "cpu_hazard_unit.vh"

module cpu #(
    parameter XLEN = 32,
    parameter MEI_PORTS = 4
) (
    input wire clk,
    input wire rst_n,

    output wire [XLEN-1:0] instr_addr,
    input  wire [XLEN-1:0] instr_data,

    output wire [XLEN-1:0] data_addr,
    output wire [XLEN-1:0] data_wdata,
    output wire [     3:0] data_wenable,
    input  wire [XLEN-1:0] data_rdata,

    input wire mti_pending,
    input wire mei_pending
);

  wire [1:0] forward_a_e;
  wire [1:0] forward_b_e;
  wire stall_f;
  wire stall_d;
  wire flush_d;
  wire flush_e;
  wire flush_m;

  cpu_hazard_unit hazard_unit (
      .rs1_d(rs1_d),
      .rs2_d(rs2_d),
      .exception_d(exception_d),

      .rs1_e(rs1_e),

      .rs2_e       (rs2_e),
      .rd_e        (rd_e),
      .csrs_e      (csrs_e),
      .pc_src_e    (pc_src_e),
      .result_src_e(result_src_e),
      .csr_write_e (csr_write_e),
      .exception_e (exception_e),

      .reg_write_m(reg_write_m),
      .csr_write_m(csr_write_m),
      .rd_m       (rd_m),
      .csrs_m     (csrs_m),
      .exception_m(exception_m),

      .reg_write_w(reg_write_w),
      .csr_write_w(csr_write_w),
      .rd_w       (rd_w),
      .csrs_w     (csrs_w),

      .forward_a_e(forward_a_e),
      .forward_b_e(forward_b_e),

      .stall_f(stall_f),
      .stall_d(stall_d),

      .flush_d(flush_d),
      .flush_e(flush_e),
      .flush_m(flush_m),

      .int_ack(int_ack)
  );

  // 1. Fetch
  reg [XLEN-1:0] pc_next_f;

  always @(*) begin
    if (int_ack) begin
      pc_next_f = mtvec_d;
    end else begin
      // NOTE: it's okay to ignore branch prediction when pc_src_e != pc + 4
      // because this situation means that the instruction causing the
      // prediction is about to get flushed, anyways.
      case (pc_src_e)
        `PC_SRC_PC_PLUS_4: begin
          if (branch_pred_taken_f && branch_target_hit_f) begin
            pc_next_f = branch_target_addr_f;
          end else begin
            pc_next_f = pc_plus_4_f;
          end
        end
        `PC_SRC_PC_TARGET:   pc_next_f = pc_target_e;
        `PC_SRC_ALU:         pc_next_f = alu_result_e;
        `PC_SRC_MTVEC:       pc_next_f = mtvec_e;
        `PC_SRC_MEPC:        pc_next_f = mepc_e;
        `PC_SRC_PC_PLUS_4_E: pc_next_f = pc_plus_4_e;
        default:             pc_next_f = {XLEN{1'bx}};
      endcase
    end
  end

  reg [XLEN-1:0] pc_f;

  always @(posedge clk) begin
    if (!rst_n) begin
      pc_f <= 0;
    end else if (!stall_f) begin
      pc_f <= pc_next_f;
    end
  end

  assign instr_addr = pc_f;
  wire [XLEN-1:0] instr_f = instr_data;

  wire [XLEN-1:0] pc_plus_4_f = pc_f + 4;

  wire branch_pred_taken_f;

  cpu_branch_predictor branch_predictor (
      .clk  (clk),
      .rst_n(rst_n),

      .update_addr (pc_e),
      .update_taken(branch_cond_val_e),
      .update      (branch_e),

      .branch_addr (pc_f),
      .branch_taken(branch_pred_taken_f)
  );

  wire            branch_target_hit_f;
  wire [XLEN-1:0] branch_target_addr_f;

  cpu_branch_target_buffer branch_target_buffer (
      .clk  (clk),
      .rst_n(rst_n),

      .update_addr       (pc_e),
      .update_target_addr(pc_target_e),
      .update            (branch_e),

      .branch_addr       (pc_f),
      .branch_hit        (branch_target_hit_f),
      .branch_target_addr(branch_target_addr_f)
  );

  // 2. Decode
  reg bubble_d;

  reg branch_pred_taken_d;

  reg [XLEN-1:0] instr_d;
  reg [XLEN-1:0] pc_d;
  reg [XLEN-1:0] pc_plus_4_d;

  always @(posedge clk) begin
    if (!rst_n || flush_d) begin
      bubble_d            <= 1;

      branch_pred_taken_d <= 0;

      instr_d             <= 32'h0000_0013;  // nop
      pc_d                <= {XLEN{1'bx}};
      pc_plus_4_d         <= {XLEN{1'bx}};
    end else if (!stall_d) begin
      bubble_d            <= 0;

      branch_pred_taken_d <= branch_pred_taken_f;

      instr_d             <= instr_f;
      pc_d                <= pc_f;
      pc_plus_4_d         <= pc_plus_4_f;
    end
  end

  wire [ 6:0] op_d = instr_d[6:0];
  wire [ 2:0] funct3_d = instr_d[14:12];
  wire [ 6:0] funct7_d = instr_d[31:25];
  wire [11:0] funct12_d = instr_d[31:20];

  wire        reg_write_d;
  wire [ 2:0] result_src_d;
  wire [ 3:0] mem_write_d;
  wire        jump_d;
  wire        branch_d;
  wire [ 3:0] alu_control_d;
  wire        alu_src_a_d;
  wire [ 1:0] alu_src_b_d;
  wire [ 2:0] imm_src_d;
  wire [ 2:0] data_ext_control_d;
  wire [ 1:0] jump_src_d;
  wire [ 2:0] branch_cond_d;
  wire        csr_write_d;
  wire        control_exception_d;
  wire [ 1:0] control_excause_d;
  wire        mret_d;
  wire [ 1:0] required_priv_d;

  cpu_control control (
      .op     (op_d),
      .funct3 (funct3_d),
      .funct7 (funct7_d),
      .funct12(funct12_d),

      .reg_write       (reg_write_d),
      .result_src      (result_src_d),
      .mem_write       (mem_write_d),
      .jump            (jump_d),
      .branch          (branch_d),
      .alu_control     (alu_control_d),
      .alu_src_a       (alu_src_a_d),
      .alu_src_b       (alu_src_b_d),
      .imm_src         (imm_src_d),
      .data_ext_control(data_ext_control_d),
      .jump_src        (jump_src_d),
      .branch_cond     (branch_cond_d),
      .csr_write       (csr_write_d),
      .exception       (control_exception_d),
      .exception_cause (control_excause_d),
      .mret            (mret_d),
      .required_priv   (required_priv_d)
  );

  wire [XLEN-1:0] rd1_d;
  wire [XLEN-1:0] rd2_d;


  wire [4:0] rs1_d = instr_d[19:15];
  wire [4:0] rs2_d = instr_d[24:20];
  wire [4:0] rd_d = instr_d[11:7];

  cpu_register_file #(
      .XLEN(XLEN)
  ) reg_file (
      .clk(~clk),

      .a1(rs1_d),
      .a2(rs2_d),

      .rd1(rd1_d),
      .rd2(rd2_d),

      .a3 (rd_w),
      .wd3(result_w),
      .we3(reg_write_w)
  );

  wire [XLEN-1:0] imm_ext_d;

  cpu_imm_extend #(
      .XLEN(XLEN)
  ) imm_ext (
      .data   (instr_d[31:7]),
      .imm_src(imm_src_d),
      .imm_ext(imm_ext_d)
  );

  wire [11:0] csrs_d = instr_d[31:20];

  wire [XLEN-1:0] csrd_d;
  wire [XLEN-1:0] mtvec_d;
  wire [XLEN-1:0] mepc_d;
  wire int_req;
  wire [1:0] priv_d;

  cpu_csr_file #(
      .XLEN(XLEN)
  ) csr_file (
      .clk  (~clk),
      .rst_n(rst_n),

      .raddr(csrs_d),
      .rdata(csrd_d),

      .waddr  (csrs_w),
      .wdata  (alu_result_w),
      .wenable(csr_write_w),

      .bubble_w(bubble_w),

      .mtvec(mtvec_d),
      .mepc (mepc_d),

      .exception_w      (exception_w),
      .exception_cause_w(exception_cause_w),
      .pc_w             (pc_w),
      .mret_w           (mret_w),

      .pc_f    (pc_f),
      .pc_d    (pc_d),
      .pc_e    (pc_e),
      .bubble_d(bubble_d),
      .bubble_e(bubble_e),

      .int_req(int_req),
      .int_ack(int_ack),

      .mti_pending(mti_pending),
      .mei_pending(mei_pending),

      .priv(priv_d)
  );

  reg int_req_buf;

  always @(posedge clk) begin
    if (!rst_n) int_req_buf <= 0;
    else int_req_buf <= int_req;
  end

  wire int_ack = int_req_buf &
    ~exception_w & ~mret_w &
    ~exception_m & ~mret_m &
    ~exception_e & ~mret_e;

  wire exception_d;
  wire [1:0] excause_d;

  cpu_exception_logic exception_logic (
      .priv         (priv_d),
      .required_priv(required_priv_d),

      .csr_write(csr_write_d),
      .csrs     (csrs_d),

      .control_exception(control_exception_d),
      .control_excause  (control_excause_d),

      .exception(exception_d),
      .excause  (excause_d)
  );

  // TODO: mask reg_write, mem_write and similar signals behind ~illegal_instr.

  // 3. Execute
  reg bubble_e;

  reg branch_pred_taken_e;
  reg reg_write_e;
  reg [2:0] result_src_e;
  reg [3:0] mem_write_e;
  reg jump_e;
  reg branch_e;
  reg [3:0] alu_control_e;
  reg alu_src_a_e;
  reg [1:0] alu_src_b_e;
  reg [2:0] data_ext_control_e;
  reg [1:0] jump_src_e;
  reg [2:0] branch_cond_e;
  reg csr_write_e;
  reg exception_e;
  reg [1:0] exception_cause_e;
  reg mret_e;

  reg [XLEN-1:0] rd1_e;
  reg [XLEN-1:0] rd2_e;
  reg [XLEN-1:0] csrd_e;
  reg [XLEN-1:0] pc_e;
  reg [4:0] rs1_e;
  reg [4:0] rs2_e;
  reg [11:0] csrs_e;
  reg [4:0] rd_e;
  reg [XLEN-1:0] imm_ext_e;
  reg [XLEN-1:0] pc_plus_4_e;

  reg [XLEN-1:0] mtvec_e;
  reg [XLEN-1:0] mepc_e;

  always @(posedge clk) begin
    if (!rst_n || flush_e) begin
      bubble_e            <= 1;

      branch_pred_taken_e <= 0;
      reg_write_e         <= 0;
      result_src_e        <= `RESULT_SRC_ALU;
      mem_write_e         <= 4'b0000;
      jump_e              <= 0;
      branch_e            <= 0;
      alu_control_e       <= 4'bxxxx;
      alu_src_a_e         <= 1'bx;
      alu_src_b_e         <= 2'bxx;
      data_ext_control_e  <= 3'bxxx;
      jump_src_e          <= 0;
      branch_cond_e       <= 3'bxxx;
      csr_write_e         <= 0;
      exception_e         <= 0;
      exception_cause_e   <= 2'bxx;
      mret_e              <= 0;

      rd1_e               <= {XLEN{1'bx}};
      rd2_e               <= {XLEN{1'bx}};
      csrd_e              <= {XLEN{1'bx}};
      pc_e                <= {XLEN{1'bx}};
      rs1_e               <= 5'bxxxxx;
      rs2_e               <= 5'bxxxxx;
      csrs_e              <= {11{1'bx}};
      rd_e                <= 5'bxxxxx;
      imm_ext_e           <= {XLEN{1'bx}};
      pc_plus_4_e         <= {XLEN{1'bx}};

      mtvec_e             <= {XLEN{1'bx}};
      mepc_e              <= {XLEN{1'bx}};
    end else begin
      bubble_e            <= bubble_d;

      branch_pred_taken_e <= branch_pred_taken_d;
      reg_write_e         <= reg_write_d;
      result_src_e        <= result_src_d;
      mem_write_e         <= mem_write_d;
      jump_e              <= jump_d;
      branch_e            <= branch_d;
      alu_control_e       <= alu_control_d;
      alu_src_a_e         <= alu_src_a_d;
      alu_src_b_e         <= alu_src_b_d;
      data_ext_control_e  <= data_ext_control_d;
      jump_src_e          <= jump_src_d;
      branch_cond_e       <= branch_cond_d;
      csr_write_e         <= csr_write_d;
      exception_e         <= exception_d;
      exception_cause_e   <= excause_d;
      mret_e              <= mret_d;

      rd1_e               <= rd1_d;
      rd2_e               <= rd2_d;
      csrd_e              <= csrd_d;
      pc_e                <= pc_d;
      rs1_e               <= rs1_d;
      rs2_e               <= rs2_d;
      csrs_e              <= csrs_d;
      rd_e                <= rd_d;
      imm_ext_e           <= imm_ext_d;
      pc_plus_4_e         <= pc_plus_4_d;

      mtvec_e             <= mtvec_d;
      mepc_e              <= mepc_d;
    end
  end

  reg [XLEN-1:0] rd1_fw_e;
  reg [XLEN-1:0] rd2_fw_e;

  always @(*) begin
    case (forward_a_e)
      `FORWARD_NONE:      rd1_fw_e = rd1_e;
      `FORWARD_WRITEBACK: rd1_fw_e = result_w;
      `FORWARD_MEMORY:    rd1_fw_e = result_pre_m;
      default:            rd1_fw_e = {XLEN{1'bx}};
    endcase

    case (forward_b_e)
      `FORWARD_NONE:      rd2_fw_e = rd2_e;
      `FORWARD_WRITEBACK: rd2_fw_e = result_w;
      `FORWARD_MEMORY:    rd2_fw_e = result_pre_m;
      default:            rd2_fw_e = {XLEN{1'bx}};
    endcase
  end

  reg [XLEN-1:0] alu_src_a_val_e;
  reg [XLEN-1:0] alu_src_b_val_e;

  always @(*) begin
    case (alu_src_a_e)
      `ALU_SRC_A_RD1: alu_src_a_val_e = rd1_fw_e;
      `ALU_SRC_A_CSR: alu_src_a_val_e = csrd_e;
      default:        alu_src_a_val_e = {XLEN{1'bx}};
    endcase

    case (alu_src_b_e)
      `ALU_SRC_B_RD2: alu_src_b_val_e = rd2_fw_e;
      `ALU_SRC_B_IMM: alu_src_b_val_e = imm_ext_e;
      `ALU_SRC_B_RD1: alu_src_b_val_e = rd1_fw_e;
      `ALU_SRC_B_RS1: alu_src_b_val_e = rs1_e;
      default:        alu_src_b_val_e = {XLEN{1'bx}};
    endcase
  end

  wire [XLEN-1:0] alu_result_e;
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

  wire [XLEN-1:0] pc_target_e = pc_e + imm_ext_e;

  wire [2:0] pc_src_e;
  wire branch_cond_val_e;

  cpu_branch_logic branch_logic (
      .jump             (jump_e),
      .jump_src         (jump_src_e),
      .branch           (branch_e),
      .branch_cond      (branch_cond_e),
      .branch_pred_taken(branch_pred_taken_e),
      .exception        (exception_e),

      .alu_carry   (alu_carry_e),
      .alu_overflow(alu_overflow_e),
      .alu_zero    (alu_zero_e),
      .alu_neg     (alu_neg_e),

      .branch_cond_val(branch_cond_val_e),
      .pc_src         (pc_src_e)
  );

  wire [XLEN-1:0] write_data_e = rd2_fw_e;

  // 4. Memory
  reg             bubble_m;

  reg             reg_write_m;
  reg  [     2:0] result_src_m;
  reg  [     3:0] mem_write_m;
  reg  [     2:0] data_ext_control_m;
  reg             csr_write_m;
  reg             exception_m;
  reg  [     1:0] exception_cause_m;
  reg             mret_m;

  reg  [XLEN-1:0] csrd_m;
  reg  [XLEN-1:0] pc_m;
  reg  [XLEN-1:0] alu_result_m;
  reg  [XLEN-1:0] write_data_m;
  reg  [    11:0] csrs_m;
  reg  [     4:0] rd_m;
  reg  [XLEN-1:0] pc_plus_4_m;
  reg  [XLEN-1:0] pc_target_m;

  always @(posedge clk) begin
    if (!rst_n || flush_m) begin
      bubble_m           <= 1;

      reg_write_m        <= 0;
      result_src_m       <= 2'bxx;
      mem_write_m        <= 4'b0000;
      data_ext_control_m <= 3'bxxx;
      csr_write_m        <= 0;
      exception_m        <= 0;
      exception_cause_m  <= 2'bxx;
      mret_m             <= 0;

      csrd_m             <= {XLEN{1'bx}};
      pc_m               <= {XLEN{1'bx}};
      alu_result_m       <= {XLEN{1'bx}};
      write_data_m       <= {XLEN{1'bx}};
      csrs_m             <= {11{1'bx}};
      rd_m               <= 5'bxxxxx;
      pc_plus_4_m        <= {XLEN{1'bx}};
      pc_target_m        <= {XLEN{1'bx}};
    end else begin
      bubble_m           <= bubble_e;
      reg_write_m        <= reg_write_e;
      result_src_m       <= result_src_e;
      mem_write_m        <= mem_write_e;
      data_ext_control_m <= data_ext_control_e;
      csr_write_m        <= csr_write_e;
      exception_m        <= exception_e;
      exception_cause_m  <= exception_cause_e;
      mret_m             <= mret_e;

      csrd_m             <= csrd_e;
      pc_m               <= pc_e;
      alu_result_m       <= alu_result_e;
      write_data_m       <= write_data_e;
      csrs_m             <= csrs_e;
      rd_m               <= rd_e;
      pc_plus_4_m        <= pc_plus_4_e;
      pc_target_m        <= pc_target_e;
    end
  end

  assign data_addr = alu_result_m;
  assign data_wdata = write_data_m;
  assign data_wenable = mem_write_m;

  wire [XLEN-1:0] read_data_m;

  cpu_data_extend data_extend (
      .data(data_rdata),
      .control(data_ext_control_m),
      .data_ext(read_data_m)
  );

  reg [XLEN-1:0] result_pre_m;

  always @(*) begin
    case (result_src_m)
      `RESULT_SRC_ALU:       result_pre_m = alu_result_m;
      `RESULT_SRC_PC_PLUS_4: result_pre_m = pc_plus_4_m;
      `RESULT_SRC_PC_TARGET: result_pre_m = pc_target_m;
      `RESULT_SRC_CSR:       result_pre_m = csrd_m;
      default:               result_pre_m = {XLEN{1'bx}};
    endcase
  end

  // 5. Writeback
  reg            bubble_w;

  reg            reg_write_w;
  reg [     2:0] result_src_w;
  reg            csr_write_w;
  reg            exception_w;
  reg [     1:0] exception_cause_w;
  reg            mret_w;

  reg [XLEN-1:0] pc_w;
  reg [XLEN-1:0] alu_result_w;
  reg [XLEN-1:0] result_pre_w;
  reg [XLEN-1:0] read_data_w;
  reg [    11:0] csrs_w;
  reg [     4:0] rd_w;
  reg [XLEN-1:0] pc_plus_4_w;
  reg [XLEN-1:0] pc_target_w;

  always @(posedge clk) begin
    if (!rst_n) begin
      bubble_w          <= 1;

      reg_write_w       <= 0;
      result_src_w      <= 2'bxx;
      csr_write_w       <= 0;
      exception_w       <= 0;
      exception_cause_w <= 2'bxx;
      mret_w            <= 0;

      pc_w              <= {XLEN{1'bx}};
      alu_result_w      <= {XLEN{1'bx}};
      result_pre_w      <= {XLEN{1'bx}};
      read_data_w       <= {XLEN{1'bx}};
      csrs_w            <= {11{1'bx}};
      rd_w              <= 5'bxxxxx;
      pc_plus_4_w       <= {XLEN{1'bx}};
      pc_target_w       <= {XLEN{1'bx}};
    end else begin
      bubble_w          <= bubble_m;

      reg_write_w       <= reg_write_m;
      result_src_w      <= result_src_m;
      csr_write_w       <= csr_write_m;
      exception_w       <= exception_m;
      exception_cause_w <= exception_cause_m;
      mret_w            <= mret_m;

      pc_w              <= pc_m;
      alu_result_w      <= alu_result_m;
      result_pre_w      <= result_pre_m;
      read_data_w       <= read_data_m;
      csrs_w            <= csrs_m;
      rd_w              <= rd_m;
      pc_plus_4_w       <= pc_plus_4_m;
      pc_target_w       <= pc_target_m;
    end
  end

  wire [XLEN-1:0] result_w = result_src_w == `RESULT_SRC_DATA ? read_data_w : result_pre_w;
endmodule
