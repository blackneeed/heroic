ENTRY(stage2_main)

SECTIONS
{
  . = 0x8000;

  .text : ALIGN(0x1000)
  {
    KEEP(*(.stage2_main))
    *(.text .text.*)
  }

  .data : ALIGN(0x1000)
  {
    *(.data .data.*)
  }

  .rodata : ALIGN(0x1000) 
  { 
    *(.rodata .rodata.*) 
  }

  .bss : ALIGN(0x1000)
  { 
    *(.bss .bss.*)
    *(COMMON)
  }
}
