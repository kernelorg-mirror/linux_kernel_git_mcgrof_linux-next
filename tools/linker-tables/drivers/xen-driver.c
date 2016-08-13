#include <asm/x86_init_fn.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <xen/xen.h>

static void early_xen_init_driver(void) {
	pr_info("Initializing xen driver\n");
	sleep(2);
}

x86_init_early_xen(early_xen_init_driver);
