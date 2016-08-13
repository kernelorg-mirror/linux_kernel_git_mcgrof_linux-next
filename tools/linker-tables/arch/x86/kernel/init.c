#define pr_fmt(fmt) "x86-init: " fmt

#include <linux/bug.h>
#include <linux/kernel.h>

#include <asm/x86_init_fn.h>
#include <asm/bootparam.h>
#include <asm/boot.h>
#include <asm/setup.h>

DEFINE_LINKTABLE_INIT_DATA(struct x86_init_fn, x86_init_fns);

static bool x86_init_fn_supports_subarch(struct x86_init_fn *fn)
{
	if (!fn->supp_hardware_subarch) {
		//pr_err("Init sequence fails to declares any supported subarchs: %pF\n", fn->early_init);
		WARN_ON(1);
	}
	if (BIT(boot_params.hdr.hardware_subarch) & fn->supp_hardware_subarch)
		return true;
	return false;
}

void __ref x86_init_fn_early_init(void)
{
	struct x86_init_fn *init_fn;
	unsigned int num_inits = LINUX_SECTION_SIZE(x86_init_fns);

	if (!num_inits)
		return;

	pr_debug("Number of init entries: %d\n", num_inits);

	LINKTABLE_FOR_EACH(init_fn, x86_init_fns) {
		if (!x86_init_fn_supports_subarch(init_fn))
			continue;

		//pr_debug("Running early init %pF ...\n", init_fn->early_init);
		init_fn->early_init();
		//pr_debug("Completed early init %pF\n", init_fn->early_init);
	}
}
