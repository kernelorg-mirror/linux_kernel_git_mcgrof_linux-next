#include <linux/kernel.h>
#include <asm/x86_init_fn.h>

static void early_init_beta(void) {
	pr_info("Initializing beta ...\n");
	pr_info("Completed initializing beta !\n");
}

x86_init_early_pc(early_init_beta);
