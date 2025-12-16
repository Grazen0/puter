.text

.global read_sp, read_mstatus, read_mcycle, read_mcause, read_mepc, inc_mepc, mstatus_set, mie_set

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

mstatus_set:
    csrs    mstatus, a0
    ret

mie_set:
    csrs    mie, a0
    ret
