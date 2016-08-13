#ifndef __PS_CONST
#define __PS_CONST

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/tables.h>

/* Helpers for partially static settings */

enum ps_static_type {
	SET_CONST_U8 = 0,
	SET_CONST_U16,
	SET_CONST_U32,
};

struct ps_set_const {
	unsigned int *count;
	enum ps_static_type type;
	unsigned int (*func)(void);
};

DECLARE_LINKTABLE(struct ps_set_const, ps_set_const_table);

#ifdef CONFIG_HAVE_ARCH_PS_CONST
#include <asm/ps_const.h>
#endif

/*
 * ps_ stands for "partially static", so we "partialloy static shift right"
 * You can optimize this for your architecture.
 *
 * ps_shr(unsigned int in, unsigned char (*func)(void))
 */
#ifndef ps_shr
#define ps_shr(_in, _func)						\
({									\
	static unsigned int _count;					\
	static LINKTABLE_INIT_DATA(ps_set_const_table, 01)		\
		__ps_shr##__func =					\
		{ &_count, SET_CONST_U8, (_func) };			\
									\
	(_in) >> _count;						\
})
#endif /* ps_shr */

#endif /* __PS_CONST */
