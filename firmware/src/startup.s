.section .text.init
.global _start

_start:
    la      sp, _stack_top
    la      gp, __global_pointer$

    la      t0, trap_handler
    csrw    mtvec, t0

    j       skip

    # Initialize .data
    la      t0, _data_start
    la      t1, _data_end
    la      t2, _text_end
init_data:
    bgeu    t0, t1, data_done

    lw      t3, 0(t2)
    addi    t2, t2, 4 # reordered to prevent lw stall
    sw      t3, 0(t0)
    addi    t0, t0, 4

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

skip:
    csrs    mstatus, 0x8 # Enable machine interrupts
    csrs    mie, 0x8     # Enable MSIs

    csrs    mip, 0x8     # Trigger an MSI

    call    __libc_init_array
    call    main
    j       .
