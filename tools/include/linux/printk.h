#ifndef __TOOLS_KERNEL_PRINTK__
#define __TOOLS_KERNEL_PRINTK__

#include <stdio.h>

#ifndef pr_fmt
#define pr_fmt(fmt)	fmt
#endif

#define pr_info(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#define pr_debug(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)

#endif /* __TOOLS_KERNEL_PRINTK__ */
