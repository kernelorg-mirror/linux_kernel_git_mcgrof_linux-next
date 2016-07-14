#ifndef _ASM_ARM_SECTIONS_H
#define _ASM_ARM_SECTIONS_H

#include <asm-generic/sections.h>

#if defined(__KERNEL__) && !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__)
extern char _exiprom[];
#endif /* defined(__KERNEL__) && !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__) */

#endif	/* _ASM_ARM_SECTIONS_H */
