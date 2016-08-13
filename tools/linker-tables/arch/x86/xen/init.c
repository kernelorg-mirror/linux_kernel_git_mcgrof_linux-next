#include <linux/kernel.h>
#include <linux/tables.h>
#include <asm/x86_init_fn.h>
#include <asm/x86.h>

void startup_xen(void)
{
	pr_info("Initializing Xen guest\n");

	x86_init_fn_early_init();

	x86_64_start_reservations();
}
