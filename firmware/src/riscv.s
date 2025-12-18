.text

.global rv_sp_read, rv_mstatus_read, rv_mcycle_read, rv_mcause_read, rv_mepc_read, rv_mepc_inc, rv_mstatus_set, rv_mie_set, rv_jump_umode

rv_sp_read:
    mv      a0, sp
    ret

rv_mstatus_read:
    csrr    a0, mstatus
    ret


rv_mcycle_read:
    csrr    a0, mcycle
    csrr    a1, mcycleh
    ret

rv_mcause_read:
    csrr    a0, mcause
    ret

rv_mepc_read:
    csrr    a0, mepc
    ret

rv_mepc_inc:
    csrr    t0, mepc
    addi    t0, t0, 4
    csrw    mepc, t0
    ret

rv_mstatus_set:
    csrs    mstatus, a0
    ret

rv_mie_set:
    csrs    mie, a0
    ret

rv_jump_umode:
    # set mstatus.MPP = U-mode (00)
    li      t0, 0x00001800
    csrc    mstatus, t0

    csrw    mepc, a0
    mret
