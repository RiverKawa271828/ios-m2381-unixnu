ENTRY(_head)
SECTIONS
{
	. = 0;
	.text : { *(.text.head) *(.text*) }
	.rodata : { *(.rodata*) }
	.data : { *(.data*) }
	. = ALIGN(16);
	_bss_start = .;
	.bss : { *(.bss*) *(COMMON) }
	. = ALIGN(16);
	_bss_end = .;
	. += 0x8000;
	. = ALIGN(16);
	_stack_top = .;
	. = ALIGN(4096);
	_sec_stacks = .;
	. += 8 * 0x1000;
	_end = .;
	/* build.sh appends the xnu payload here (Meizu GKI ABL ignores the
	 * boot.img ramdisk); loader_boot checks the magic before trusting it. */
	. = ALIGN(0x4000);
	_payload_start = .;
	/DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
