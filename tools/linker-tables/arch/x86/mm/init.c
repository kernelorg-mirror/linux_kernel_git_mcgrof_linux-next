#include <linux/kernel.h>
#include <asm/x86_init_fn.h>

static void early_init_memory(void) {
	pr_info("Initializing memory ...\n");
	sleep(1);
	pr_info("Completed initializing memory !\n");
}

x86_init_early_all(early_init_memory);
