#include <linux/kernel.h>
#include <linux/tables.h>
#include <asm/x86_init_fn.h>
#include <linux/ranges.h>
#include <linux/kprobes.h>

DEFINE_SECTION_RANGE(kprobes, SECTION_TEXT);

void __kprobes test_kprobe_0001(void)
{
	pr_info("test_kprobe\n");
}

void test_kprobe_0002(void)
{
	pr_info("test_kprobe\n");
}

void test_kprobe_addr(const char *test, unsigned long addr, bool should_match)
{
	if (SECTION_ADDR_IN_RANGE(kprobes, addr))
		if (should_match)
			pr_info("== OK: %s within range!\n", test);
		else
			pr_info("== FAIL: %s should not be in range...\n",
				test);
	else
		if (should_match)
			pr_info("== FAIL: %s should be in range...\n", test);
		else
			pr_info("== OK: %s not in range as expected!\n", test);
}

void early_init_kprobes(void)
{
	unsigned long addr;

	pr_info("Initializing kprobes ...\n");

	addr = (unsigned long) &test_kprobe_0001;

	test_kprobe_addr("test_kprobe_0001", addr, true);

	addr = (unsigned long) &test_kprobe_0002;

	test_kprobe_addr("test_kprobe_0002", addr, false);

	pr_info("Completed initializing kprobes !\n");
}

x86_init_early_all(early_init_kprobes);
