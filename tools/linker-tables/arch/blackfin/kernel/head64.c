#include <linux/kernel.h>
#include <linux/tables.h>
#include <linux/start_kernel.h>

static void blackfin_start_kernel(void)
{
	start_kernel();
}

void startup_64(void)
{
	pr_info("Initializing x86 bare metal world\n");
	blackfin_start_kernel();
}

void setup_arch(void)
{
	/* TODO: blackfin_init_fn_setup_arch(); */
}

void late_init(void)
{
	/* TODO: blackfin_init_fn_late_init(); */
}
