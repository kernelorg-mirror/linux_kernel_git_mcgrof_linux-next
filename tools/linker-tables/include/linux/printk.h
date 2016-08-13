#ifndef _SANDBOX_KERNEL_PRINTK
#define _SANDBOX_KERNEL_PRINTK

#ifdef __KERNEL__

#include <stdio.h>

#ifndef pr_fmt
#define pr_fmt(fmt)	fmt
#endif

#ifndef pr_info
#define pr_info(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#endif

#ifndef pr_err
#define pr_err(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#endif

#ifndef pr_debug
#define pr_debug(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#endif

#endif /* __KERNEL__ */

#endif /* _SANDBOX_KERNEL_PRINTK */
