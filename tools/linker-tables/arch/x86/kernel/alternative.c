#include <asm/x86_init_fn.h>

#include <linux/kernel.h>
#include <linux/ps_const.h>

void apply_alternatives_linker_tables(void)
{
	unsigned int num_consts = LINUX_SECTION_SIZE(ps_set_const_table);
	struct ps_set_const *ps_const;

	if (!num_consts)
		return;

	pr_debug("Number of init entries: %d\n", num_consts);

	LINKTABLE_FOR_EACH(ps_const, ps_set_const_table) {
		switch(ps_const->type) {
		case SET_CONST_U8:
			*ps_const->count = (__u8) ps_const->func();
			break;
		case SET_CONST_U16:
			*ps_const->count = (__u16) ps_const->func();
			break;
		case SET_CONST_U32:
			*ps_const->count = (__u16) ps_const->func();
			break;
		}
	}
}

x86_init_early_pc(apply_alternatives_linker_tables);
