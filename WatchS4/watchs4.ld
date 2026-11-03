ENTRY(module_initialize)
SECTIONS
{
    .text : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data .data.*) }
    .bss : { *(.bss .bss.* COMMON) }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
