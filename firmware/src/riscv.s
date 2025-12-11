.text

.global read_sp, read_mcycle, read_mcause, read_mepc, inc_mepc

read_sp:
    mv      a0, sp
    ret

read_mcycle:
    csrr    a0, mcycle
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
