#include <linux/kernel.h>
#include <linux/tables.h>
#include <asm/x86_init_fn.h>
#include <asm/bootparam.h>
#include <linux/pci.h>

DECLARE_LINKTABLE(struct pci_fixup, pci_fixup_early);

void early_init_pci(void)
{

	const struct pci_fixup *fixup;
	unsigned int tbl_size = LINKTABLE_SIZE(pci_fixup_early);

	pr_info("Initializing pci ...\n");

	pr_info("PCI fixup size: %d\n", tbl_size);

	sleep(1);
	pr_info("Demo: Using linktable_for_each\n");
	linktable_for_each(fixup, pci_fixup_early)
		fixup->hook();

	pr_info("Demo: Using linktable_run_all\n");
	linktable_run_all(pci_fixup_early, hook,);

	pr_info("Completed initializing pci !\n");
}

x86_init_early_all(early_init_pci);
