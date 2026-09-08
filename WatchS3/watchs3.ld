/* Compact ET_REL layout
*/
ENTRY(module_initialize)

SECTIONS
{
    .text : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data .data.*) }
    .bss : { *(.bss .bss.* COMMON) }
    
    .init_array : {
        KEEP(*(.init_array))
        KEEP(*(.init_array.*))
    }

    .fini_array : {
        KEEP(*(.fini_array))
        KEEP(*(.fini_array.*))
    }
}