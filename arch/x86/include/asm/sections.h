#ifndef _ASM_X86_SECTIONS_H
#define _ASM_X86_SECTIONS_H

#include <asm-generic/sections.h>

#if defined(__KERNEL__) && !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__)

extern char __brk_base[], __brk_limit[];

/*
 * The exception table consists of triples of addresses relative to the
 * exception table entry itself. The first address is of an instruction
 * that is allowed to fault, the second is the target at which the program
 * should continue. The third is a handler function to deal with the fault
 * caused by the instruction in the first field.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry {
	int insn, fixup, handler;
};

extern struct exception_table_entry __stop___ex_table[];

#if defined(CONFIG_X86_64)
extern char __end_rodata_hpage_align[];
#endif

#endif /* defined(__KERNEL__) && !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__) */

#endif	/* _ASM_X86_SECTIONS_H */
