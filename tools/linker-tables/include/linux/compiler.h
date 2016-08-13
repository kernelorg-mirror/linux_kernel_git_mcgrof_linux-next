#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

#include <linux/sections.h>

#define __section(S)	__attribute__ ((__section__(#S)))

#ifdef CONFIG_KPROBES
#define __kprobes	__LINUX_RANGE(SECTION_TEXT, kprobes)
#endif

#endif /* __LINUX_COMPILER_H */
