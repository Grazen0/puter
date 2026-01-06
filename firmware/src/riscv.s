.text

.global rv_read_sp
rv_read_sp:
    mv      a0, sp
    ret

.global rv_read_mstatus
rv_read_mstatus:
    csrr    a0, mstatus
    ret

.global rv_read_mcycle
rv_read_mcycle:
    csrr    a0, mcycle
    csrr    a1, mcycleh
    ret

.global rv_read_minstret
rv_read_minstret:
    csrr    a0, minstret
    csrr    a1, minstreth
    ret

.global rv_read_mcause
rv_read_mcause:
    csrr    a0, mcause
    ret

.global rv_read_mepc
rv_read_mepc:
    csrr    a0, mepc
    ret

.global rv_mepc_inc
rv_mepc_inc:
    csrr    t0, mepc
    addi    t0, t0, 4
    csrw    mepc, t0
    ret

.global rv_mstatus_set
rv_mstatus_set:
    csrs    mstatus, a0
    ret

.global rv_mie_set
rv_mie_set:
    csrs    mie, a0
    ret

.global rv_jump_umode
rv_jump_umode:
    # set mstatus.MPP = U-mode (00)
    li      t0, 0x00001800
    csrc    mstatus, t0

    csrw    mepc, a0
    mret
