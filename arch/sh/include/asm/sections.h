#ifndef __ASM_SH_SECTIONS_H
#define __ASM_SH_SECTIONS_H

#include <asm-generic/sections.h>

#if defined(__KERNEL__) && !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__)
extern long __machvec_start, __machvec_end;
extern char __uncached_start, __uncached_end;
extern char __start_eh_frame[], __stop_eh_frame[];
#endif /* defined(__KERNEL__) && !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__) */

#endif /* __ASM_SH_SECTIONS_H */

