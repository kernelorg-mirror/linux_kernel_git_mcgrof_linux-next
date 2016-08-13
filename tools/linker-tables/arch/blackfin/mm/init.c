#include <linux/kernel.h>
#include <asm-generic/arch_init_fn.h>

static void early_init_memory(void) {
	pr_info("Initializing memory ...\n");
	sleep(1);
	pr_info("Completed initializing memory !\n");
}

arch_init_early(early_init_memory);
