#include <linux/kernel.h>
#include <linux/tables.h>
#include <asm/x86_init_fn.h>
#include <asm/bootparam.h>
#include <linux/pci.h>

DECLARE_LINKTABLE(struct pci_fixup, pci_fixup_early);

void early_init_pci(void) {

	const struct pci_fixup *fixup;
	unsigned int tbl_size = LINUX_SECTION_SIZE(pci_fixup_early);

	pr_info("Initializing pci ...\n");

	pr_info("PCI fixup size: %d\n", tbl_size);

	sleep(1);
	pr_info("Demo: Using LINKTABLE_FOR_EACH\n");
	LINKTABLE_FOR_EACH(fixup, pci_fixup_early)
		fixup->hook();

	pr_info("Demo: Using LINKTABLE_RUN_ALL\n");
	LINKTABLE_RUN_ALL(pci_fixup_early, hook,);

	pr_info("Completed initializing pci !\n");
}

x86_init_early_all(early_init_pci);
