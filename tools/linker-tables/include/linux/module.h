#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <linux/init.h>

#define module_init(x)  __initcall(x);
#define module_exit(x)  __exitcall(x);

struct module {
	int (*init)(void);
	void (*exit)(void);
};

#endif /* _LINUX_MODULE_H */
