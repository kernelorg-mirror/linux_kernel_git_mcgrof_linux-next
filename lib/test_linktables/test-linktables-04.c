#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/tables.h>
#include <linux/printk.h>

#include "test-linktables.h"

static int __init test_linktable_04_init(int input)
{
	return 4 * input;
}

static int __initdata test_linktable_04_init_data(int input)
{
	return 4 * input;
}

static int test_linktable_04(int input)
{
	return 4 * input;
}

test_linktable_init_data(04, test_linktable_04_init_data); /* .init.data */
test_linktable_init_text(04, test_linktable_04_init); /* .init.text */
test_linktable(04, test_linktable_04); /* .data */
test_linktable_text(04, test_linktable_04); /* .text */
test_linktable_rodata(04, test_linktable_04); /* .rodata */
