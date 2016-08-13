#include <linux/kernel.h>
#include <linux/tables.h>
#include <linux/pci.h>

DEFINE_LINKTABLE_RO(struct pci_fixup, pci_fixup_early);

static void foo_fixup(void) {
	pr_info("foo_fixup\n");
};

LINKTABLE_RO(pci_fixup_early, 50) quirk_foo = {
	.hook = foo_fixup,
};
