#include <linux/types.h>

struct pci_fixup {
	void (*hook)(void);
};

bool detect_pci(void);
