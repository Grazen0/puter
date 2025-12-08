.section .text.init
.global _start

_start:
    la      sp, _stack_top
    la      gp, __global_pointer$

    # Initialize .data
    la      t0, _data_start
    la      t1, _data_end
    la      t2, _text_end
init_data:
    bgeu    t0, t1, data_done
    lw      t3, 0(t2)
    sw      t3, 0(t0)
    addi    t0, t0, 4
    addi    t2, t2, 4
    j       init_data
data_done:

    # Initialize .bss
    la      t0, _bss_start
    la      t1, _bss_end
clear_bss:
    bgeu    t0, t1, bss_done
    sw      zero, 0(t0)
    addi    t0, t0, 4
    j       clear_bss
bss_done:

    call    __libc_init_array
    call    main
    j       .
