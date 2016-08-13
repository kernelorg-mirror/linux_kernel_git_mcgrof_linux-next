#include <linux/kernel.h>
#include <linux/tables.h>

#include <asm/x86_init_fn.h>
#include <asm/boot.h>
#include <asm/bootparam.h>

#include <linux/start_kernel.h>
#include <linux/kasan.h>

void x86_64_start_reservations(void)
{
	switch (boot_params.hdr.hardware_subarch) {
	case X86_SUBARCH_PC:
		pr_info("Booting bare metal\n");
		break;
	case X86_SUBARCH_LGUEST:
		pr_info("Booting lguest not supported\n");
		BUG();
	case X86_SUBARCH_XEN:
		pr_info("Booting a Xen guest\n");
		break;
	case X86_SUBARCH_INTEL_MID:
		pr_info("Booting Intel MID not supported\n");
		BUG();
	case X86_SUBARCH_CE4100:
		pr_info("Booting Intel CE4100 not supported\n");
		BUG();
	default:
		pr_info("Booting sunsupported x86 hardware subarch\n");
		BUG();
	}

	start_kernel();
}

static void x86_64_start_kernel(void)
{
	x86_init_fn_early_init();

	x86_64_start_reservations();
}

void startup_64(void)
{
	pr_info("Initializing x86 bare metal world\n");
	x86_64_start_kernel();
}

void setup_arch(void)
{
	/* TODO: x86_init_fn_setup_arch(); */
}

void late_init(void)
{
	/* TODO: x86_init_fn_late_init(); */
}
