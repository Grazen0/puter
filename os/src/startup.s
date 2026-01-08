.section .text.init
.global _start

_start:
    la      sp, _stack_top
    la      gp, __global_pointer$

    la      t0, trap_handler
    csrw    mtvec, t0

    # .data was already copied by firmware

    # Initialize .bss
    la      t0, _bss_start
    la      t1, _bss_end
clear_bss:
    sw      zero, 0(t0)
    addi    t0, t0, 4
    blt     t0, t1, clear_bss

    call    main
    j       .
