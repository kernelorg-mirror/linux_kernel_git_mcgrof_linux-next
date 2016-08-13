#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/x86_init_fn.h>
#include <asm/x86.h>

DEFINE_LINKTABLE_INIT_DATA(initcall_t, init_calls);

int do_one_initcall(initcall_t fn)
{
	int ret;

	ret = fn();

	return ret;
}

static void do_initcalls(void)
{
	initcall_t *fn;

	 LINKTABLE_FOR_EACH(fn, init_calls)
		 do_one_initcall(*fn);
}

void start_kernel(void)
{
	pr_info("Calling start_kernel()...\n");

	setup_arch();
	late_init();
	do_initcalls();
}
