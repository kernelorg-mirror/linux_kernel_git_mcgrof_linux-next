#include <linux/kernel.h>
#include <asm/x86_init_fn.h>

void kasan_early_init(void) {
	pr_info("Initializing kasan ...\n");
	pr_info("Early init for Kasan...\n");
	pr_info("Completed initializing kasan !\n");
}

x86_init_early_pc(kasan_early_init);
