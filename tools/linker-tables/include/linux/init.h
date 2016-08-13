#ifndef _SANDBOX_LINUX_INIT_H
#define _SANDBOX_LINUX_INIT_H

#include_next <linux/init.h>
#include <linux/types.h>
#include <linux/sections.h>

#define __init	__section(SECTION_INIT)
#define __exit	__section(SECTION_EXIT)

#ifndef __ASSEMBLY__
#include <linux/tables.h>
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);

DECLARE_LINKTABLE(initcall_t, init_calls);
DECLARE_LINKTABLE(exitcall_t, exit_calls);

#define __define_initcall(fn, id)					\
	static LINKTABLE_INIT_DATA(init_calls, id)			\
	__initcall_##fn##id = fn

#define pure_initcall(fn)			__define_initcall(fn, 0)
#define core_initcall(fn)			__define_initcall(fn, 1)
#define postcore_initcall(fn)			__define_initcall(fn, 2)
#define arch_initcall(fn)			__define_initcall(fn, 3)
#define subsys_initcall(fn)			__define_initcall(fn, 4)
#define fs_initcall(fn)				__define_initcall(fn, 5)
#define device_initcall(fn)			__define_initcall(fn, 6)
#define late_initcall(fn)			__define_initcall(fn, 7)

#define __initcall(fn)				device_initcall(fn)

#define __exitcall(fn)							\
	static LINKTABLE_INIT_DATA(exit_calls, SECTION_ORDER_ANY)	\
	__exitcall_##fn = fn;

#endif

#endif /* _SANDBOX_LINUX_INIT_H */
