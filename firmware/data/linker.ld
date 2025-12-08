OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY {
  ROM (rx) : ORIGIN = 0x00000000, LENGTH = 32K
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 32K
}

SECTIONS {
  .text : {
    . = ALIGN(4);

    KEEP(*(.text.init))
    *(.text*)

    __preinit_array_start = .;
    KEEP(*(.pre_init_array*))
    __preinit_array_end = .;

    __init_array_start = .;
    KEEP(*(.init_array*))
    __init_array_end = .;

    KEEP(*(.fini_array*))
    *(.fini*)

    *(.rodata*)

    . = ALIGN(4);
    _text_end = .;
  } > ROM

  .data : {
    . = ALIGN(4);
    _data_start = .;

    *(.data*)
    __global_pointer$ = . + 0x800;
    *(.sdata*)

    . = ALIGN(4);
    _data_end = .;
  } > RAM AT > ROM

  .bss : {
    . = ALIGN(4);
    _bss_start = .;

    *(.bss*)
    *(COMMON)
    *(.sbss*)

    . = ALIGN(4);
    _bss_end = .;
  } > RAM

  . = ALIGN(4);
  _end = .;

  _stack_size = 4K;
  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
  _stack_bottom = _stack_top - _stack_size;

  ASSERT(_end <= _stack_bottom, "Error: Heap collides with stack")
}
