ENTRY(_start)

MEMORY {
  RAM (rwx) : ORIGIN = 0x80000800, LENGTH = 30K
}

SECTIONS {
  .text : {
    KEEP(*(.text.init))
    *(.text*)
    *(.rodata*)

    . = ALIGN(4);
    _text_end = .;
  } > RAM

  .data : {
    . = ALIGN(4);
    _data_start = .;

    *(.data*)
    __global_pointer$ = . + 0x800;
    *(.sdata*)

    . = ALIGN(4);
    _data_end = .;
  } > RAM

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

  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
