#ifndef __X86_PS_CONST
#define __X86_PS_CONST

#include <linux/tables.h>
#include <asm/asm.h>

#define ps_shr(_in, _func)						\
({									\
	__typeof__(_in) _out;						\
	asm volatile(							\
		"shr $0,%0\n"						\
		"1:\n"							\
		SECTION_TBL_STR(SECTION_INIT_DATA, ps_set_const_table, 01)\
		_ASM_PTR "1b-1, %P2, %P3\n"				\
		".popsection\n"						\
		: "=rm" (_out)						\
		: "0" (_in), "i" (SET_CONST_U8), "i" (_func));		\
	(_out);								\
})

#endif /* __X86_PS_CONST */
