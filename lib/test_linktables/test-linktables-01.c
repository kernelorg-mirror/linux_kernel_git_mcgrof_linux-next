#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/tables.h>
#include <linux/printk.h>

#include "test-linktables.h"

static int __init test_linktable_01_init(int input)
{
	return 1 * input;
}

static int __initdata test_linktable_01_init_data(int input)
{
	return 1 * input;
}

static int test_linktable_01(int input)
{
	return 1 * input;
}

test_linktable_init_data(01, test_linktable_01_init_data); /* .init.data */
test_linktable_init_text(01, test_linktable_01_init); /* .init.text */
test_linktable(01, test_linktable_01); /* .data */
test_linktable_text(01, test_linktable_01); /* .text */
test_linktable_rodata(01, test_linktable_01); /* .rodata */
