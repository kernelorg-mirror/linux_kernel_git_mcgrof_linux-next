#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/types.h>

/* no equivalent on userspace yet */
#define __ref

#ifndef pr_fmt
#define pr_fmt(fmt)	fmt
#endif

#define pr_info(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)
#define pr_debug(fmt, ...)	printf(pr_fmt(fmt), ##__VA_ARGS__)

#define BUG() do {										\
	fprintf(stderr, "----------------------------------------------------------\n");	\
	fprintf (stderr, "BUG on %s at %s: %i\n", __func__, __FILE__, __LINE__);		\
	abort();										\
}												\
while (0)

#define BUG_ON(cond) do { if (cond) BUG(); } while (0)

#define WARN_ON(__test) do {									\
	if (__test) {										\
		fprintf(stderr, "----------------------------------------------------------\n");\
		fprintf (stderr, "WARN_ON on %s at %s: %i\n", __func__, __FILE__, __LINE__);	\
	}											\
}												\
while (0)

#define __used		__attribute__((__used__))

#endif /* _LINUX_KERNEL_H */
