#define pr_fmt(fmt) "Synthetics: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ps_const.h>

#include "common.h"
#include "synth.h"

DEFINE_LINKTABLE_INIT_DATA(struct ps_set_const, ps_set_const_table);

unsigned int get_demo_shr(void)
{
	return 16;
}

static int synth_init(void)
{
	int synth_or;
	int val = 2;
	const unsigned int reg =  ps_shr(0xDEADBEEF, get_demo_shr);

	synth_or = synth_init_or(val);
	pr_info("synth_init_or(%d) returns: 0x%08X\n", val, synth_or);

	pr_info("ps_shr(0x%08X, get_demo_shr) returns: 0x%08X\n", 0xDEADBEEF, reg);

	return 0;
}
module_init(synth_init);

static void synth_exit(void)
{
}
module_exit(synth_exit);
