#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/tables.h>
#include <linux/printk.h>

#include "test-linktables.h"

static int __init test_linktable_00_init(int input)
{
	return 0 * input;
}

static int __initdata test_linktable_00_init_data(int input)
{
	return 0 * input;
}

static int test_linktable_00(int input)
{
	return 0 * input;
}

test_linktable_init_data(00, test_linktable_00_init_data); /* .init.data */
test_linktable_init_text(00, test_linktable_00_init); /* .init.text */
test_linktable(00, test_linktable_00); /* .data */
test_linktable_text(00, test_linktable_00); /* .text */
test_linktable_rodata(00, test_linktable_00); /* .rodata */
