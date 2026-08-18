ENTRY(stage2_main)

SECTIONS
{
  . = 0x8000;

  .text :
  {
    KEEP(*(.stage2_main))
    *(.text .text.*)
  }

  .data :
  {
    *(.data .data.*)
  }

  .rodata :
  {
    *(.rodata .rodata.*)
  }

  .bss :
  {
    *(.bss .bss.*)
    *(COMMON)
  }
}
