#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/tables.h>
#include <linux/printk.h>

#include "test-linktables.h"

static int __init test_linktable_03_init(int input)
{
	return 3 * input;
}

static int __initdata test_linktable_03_init_data(int input)
{
	return 3 * input;
}

static int test_linktable_03(int input)
{
	return 3 * input;
}

test_linktable_init_data(03, test_linktable_03_init_data); /* .init.data */
test_linktable_init_text(03, test_linktable_03_init); /* .init.text */
test_linktable(03, test_linktable_03); /* .data */
test_linktable_text(03, test_linktable_03); /* .text */
test_linktable_rodata(03, test_linktable_03); /* .rodata */
