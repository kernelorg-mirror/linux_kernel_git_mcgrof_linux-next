#ifndef _SANDBOX_LINUX_KPROBES_H
#define _SANDBOX_LINUX_KPROBES_H

#include <asm/kprobes.h>
#include <linux/ranges.h>

#ifdef CONFIG_KPROBES
DECLARE_SECTION_RANGE(kprobes);
#endif

#endif /* _SANDBOX_LINUX_KPROBES_H */
