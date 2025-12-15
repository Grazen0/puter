.text

.global read_sp, read_mstatus, read_mcycle, read_mcause, read_mepc, inc_mepc, enable_mti

read_sp:
    mv      a0, sp
    ret

read_mstatus:
    csrr    a0, mstatus
    ret


read_mcycle:
    csrr    a0, mcycle
    csrr    a1, mcycleh
    ret

read_mcause:
    csrr    a0, mcause
    ret

read_mepc:
    csrr    a0, mepc
    ret

inc_mepc:
    csrr    t0, mepc
    addi    t0, t0, 4
    csrw    mepc, t0
    ret

enable_mti:
    li      t0, 0x800
    csrs    mie, t0
    csrs    mstatus, 0x8
    ret
