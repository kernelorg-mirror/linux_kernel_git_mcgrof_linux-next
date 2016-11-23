#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/tables.h>
#include <linux/printk.h>

#include "test-linktables.h"

static int __init test_linktable_02_init(int input)
{
	return 2 * input;
}

static int __initdata test_linktable_02_init_data(int input)
{
	return 2 * input;
}

static int test_linktable_02(int input)
{
	return 2 * input;
}

test_linktable_init_data(02, test_linktable_02_init_data); /* .init.data */
test_linktable_init_text(02, test_linktable_02_init); /* .init.text */
test_linktable(02, test_linktable_02); /* .data */
test_linktable_text(02, test_linktable_02); /* .text */
test_linktable_rodata(02, test_linktable_02); /* .rodata */
