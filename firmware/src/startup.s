.section .text.init
.global _start

_start:
    la      sp, _stack_top
    la      gp, __global_pointer$

    la      t0, trap_handler
    csrw    mtvec, t0

    # Initialize .data
    la      t0, _data_start
    la      t1, _data_end
    la      t2, _text_end
init_data:
    lw      t3, 0(t2)
    addi    t2, t2, 4 # reordered to prevent lw stall
    sw      t3, 0(t0)
    addi    t0, t0, 4
    blt     t0, t1, init_data

    # Initialize .bss
    la      t0, _bss_start
    la      t1, _bss_end
clear_bss:
    sw      zero, 0(t0)
    addi    t0, t0, 4
    blt     t0, t1, clear_bss

    call    __libc_init_array
    call    main
    j       .
