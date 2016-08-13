#ifndef __ARCH_INIT_TABLES_H
#define __ARCH_INIT_TABLES_H

#include <linux/types.h>
#include <linux/tables.h>
#include <linux/init.h>

/**
 * struct arch_init_fn - architecture-generic kernel init call
 *
 * Architectures must initialize a series of things prior to handing off
 * control to the kernel. This structure can be used if the architecture is
 * simply and it just needs a basic set of calls on its way up.
 *
 * @early_init: required, routine which will run in startup_64(). Memory is
 * 	not yet available.
 */
struct arch_init_fn {
	void (*early_init)(void);
};

DECLARE_LINKTABLE(struct arch_init_fn, arch_init_fns);

/* Init order levels, we can start at 0000 but reserve 0000-0999 for now */

/*
 * ARCH_INIT_ORDER_EARLY - early kernel init code
 *
 * This consists of the first parts of the Linux kernel executed.
 */
#define ARCH_INIT_ORDER_EARLY	1000

#define __arch_init(__level,						\
		    __early_init)					\
	static LINKTABLE_INIT_DATA(arch_init_fns, __level)		\
	__arch_init_fn_##__early_init = {				\
		.early_init = __early_init,				\
	}

#define arch_init_early(__early_init)					\
	__arch_init(ARCH_INIT_ORDER_EARLY, __early_init)

/**
 * arch_init_fn_early_init: call all early_init() callbacks
 *
 * This calls all early_init() callbacks on the arch_init_fns linker table.
 */
void arch_init_fn_early_init(void);

#endif /* __ARCH_INIT_TABLES_H */
