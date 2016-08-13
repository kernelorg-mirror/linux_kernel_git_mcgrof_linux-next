#ifndef __X86_PS_CONST
#define __X86_PS_CONST

#include <linux/stringify.h>
#include <linux/tables.h>
#include <asm/asm.h>

#define ps_shr(in, _func)						\
({									\
	 __typeof__(in) _count;						\
									\
	asm volatile(							\
		"shr %P[_in],%[_count]\n"				\
		"1:\n"							\
		push_section_tbl(.init.data, ps_set_const_table, 01,)	\
		_ASM_PTR "1b-1, %P2, %P3\n"				\
		".popsection\n"						\
		: [_count] "=g" (_count)				\
		: [_in] "i" (in), "i" (SET_CONST_U8), "i" (_func));	\
	(_count);							\
})

#endif /* __X86_PS_CONST */
