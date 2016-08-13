#include <linux/kernel.h>
#include <asm/x86_init_fn.h>

static void early_init_alpha(void) {
	pr_info("Initializing alpha ...\n");
	pr_info("Completed initializing alpha !\n");
}

x86_init_early_pc(early_init_alpha);
