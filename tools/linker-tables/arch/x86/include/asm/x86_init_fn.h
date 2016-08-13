#ifndef __X86_INIT_TABLES_H
#define __X86_INIT_TABLES_H

#include <linux/types.h>
#include <linux/tables.h>

#include <linux/init.h>
#include <linux/bitops.h>
#include <asm/bootparam.h>

/**
 * struct x86_init_fn - x86 generic kernel init call
 *
 * Linux x86 features vary in complexity, features may require work done at
 * different levels of the full x86 init sequence. Today there are also two
 * different possible entry points for Linux on x86, one for bare metal, KVM
 * and Xen HVM, and another for Xen PV guests / dom0.  Assuming a bootloader
 * has set up 64-bit mode, roughly the x86 init sequence follows this path:
 *
 * Bare metal, KVM, Xen HVM                      Xen PV / dom0
 *       startup_64()                             startup_xen()
 *              \                                     /
 *      x86_64_start_kernel()                 xen_start_kernel()
 *                           \               /
 *                      x86_64_start_reservations()
 *                                   |
 *                              start_kernel()
 *                              [   ...        ]
 *                              [ setup_arch() ]
 *                              [   ...        ]
 *                                  init
 *
 * x86_64_start_kernel() and xen_start_kernel() are the respective first C code
 * entry starting points. The different entry points exist to enable Xen to
 * skip a lot of hardware setup already done and managed on behalf of the
 * hypervisor, we refer to this as "paravirtualization yielding". The different
 * levels of init calls on the x86 init sequence exist to account for these
 * slight differences and requirements. These different entry points also share
 * a common entry x86 specific path, x86_64_start_reservations().
 *
 * A generic x86 feature can have different initialization calls, one on each
 * of the different main x86 init sequences, but must also address both entry
 * points in order to work properly across the board on all supported x86
 * subarchitectures. Since x86 features can also have dependencies on other
 * setup code or features, x86 features can at times be subordinate to other
 * x86 features, or conditions. struct x86_init_fn enables feature developers
 * to annotate dependency relationships to ensure subsequent init calls only
 * run once a subordinate's dependencies have run. When needed custom
 * dependency requirements can also be spelled out through a custom dependency
 * checker. In order to account for the dual entry point nature of x86-64 Linux
 * for "paravirtualization yielding" and to make annotations for support for
 * these explicit each struct x86_init_fn must specify supported
 * subarchitectures. The earliest x86-64 code can read the subarchitecture
 * though is after load_idt(), as such the earliest we can currently rely on
 * subarchitecture for semantics and a common init sequences is on the shared
 * common x86_64_start_reservations().  Each struct x86_init_fn is associated
 * with a specific special link order number which has been careflly thought
 * out by x86 maintainers. You should pick a link order level associated with
 * the specific directory your code lies in, a respective macro is used to
 * build association to a link oder with a routine, you should use one of the
 * provided x86_init_*() macros. You should not use __x86_init() directly.
 *
 * x86_init_fn enables strong semantics and dependencies to be defined and
 * implemented on the full x86 initialization sequence.
 *
 * @supp_hardware_subarch: must be set, it represents the bitmask of supported
 *	subarchitectures.  We require each struct x86_init_fn to have this set
 *	to require developer considerations for each supported x86
 *	subarchitecture and to build strong annotations of different possible
 *	run time states particularly in consideration for the two main
 *	different entry points for x86 Linux, to account for paravirtualization
 *	yielding.
 *
 *	The subarchitecture is read by the kernel at early boot from the
 *	struct boot_params hardware_subarch. Support for the subarchitecture
 *	exists as of x86 boot protocol 2.07. The bootloader would have set up
 *	the respective hardware_subarch on the boot sector as per
 *	Documentation/x86/boot.txt.
 *
 *	What x86 entry point is used is determined at run time by the
 *	bootloader. Linux pv_ops was designed to help enable to build one Linux
 *	binary to support bare metal and different hypervisors.  pv_ops setup
 *	code however is limited in that all pv_ops setup code is run late in
 *	the x86 init sequence, during setup_arch(). In fact cpu_has_hypervisor
 *	only works after early_cpu_init() during setup_arch(). If an x86
 *	feature requires an earlier determination of what hypervisor was used,
 *	or if it needs to annotate only support for certain hypervisors, the
 *	x86 hardware_subarch should be set by the bootloader and
 *	@supp_hardware_subarch set by the x86 feature. Using hardware_subarch
 *	enables x86 features to fill the semantic gap between the Linux x86
 *	entry point used and what pv_ops has to offer through a hypervisor
 *	agnostic mechanism.
 *
 *	Each supported subarchitecture is set using the respective
 *	X86_SUBARCH_* as a bit in the bitmask. For instance if a feature
 *	is supported on PC and Xen subarchitectures only you would set this
 *	bitmask to:
 *
 *		BIT(X86_SUBARCH_PC) |
 *		BIT(X86_SUBARCH_XEN);
 *
 * @early_init: required, routine which will run in x86_64_start_reservations()
 *	after we ensure boot_params.hdr.hardware_subarch is accessible and
 *	properly set. Memory is not yet available. This the earliest we can
 *	currently define a common shared callback since all callbacks need to
 *	check for boot_params.hdr.hardware_subarch and this becomes accessible
 *	on x86-64 until after load_idt().
 */
struct x86_init_fn {
	__u32 supp_hardware_subarch;
	void (*early_init)(void);
};

DECLARE_LINKTABLE(struct x86_init_fn, x86_init_fns);

/* Init order levels, we can start at 0000 but reserve 0000-0999 for now */

/*
 * X86_INIT_ORDER_EARLY - early kernel init code
 *
 * This consists of the first parts of the Linux kernel executed.
 */
#define X86_INIT_ORDER_EARLY	1000

/* X86_INIT_ORDER_PLATFORM - platform kernel code
 *
 * Code the kernel needs to initialize under arch/x86/platform/
 * early in boot.
 */
#define X86_INIT_ORDER_PLATFORM	3000

#define __x86_init(__level,						\
		   __supp_hardware_subarch,				\
		   __early_init)					\
	static LINKTABLE_INIT_DATA(x86_init_fns, __level)		\
	__x86_init_fn_##__early_init = {				\
		.supp_hardware_subarch = __supp_hardware_subarch,	\
		.early_init = __early_init,				\
	};

#define x86_init_early(__supp_hardware_subarch,				\
		       __early_init)					\
	__x86_init(X86_INIT_ORDER_EARLY, __supp_hardware_subarch,	\
		   __early_init);

#define x86_init_platform(__supp_hardware_subarch,			\
		       __early_init)					\
	__x86_init(__name, X86_INIT_ORDER_PLATFORM, __supp_hardware_subarch,\
		   __early_init);

#define x86_init_early_all(__early_init)				\
	x86_init_early(X86_SUBARCH_ALL_SUBARCHS,			\
		       __early_init);

#define x86_init_early_pc(__early_init)					\
	x86_init_early(BIT(X86_SUBARCH_PC),				\
		       __early_init);

#define x86_init_early_xen(__early_init)				\
	x86_init_early(BIT(X86_SUBARCH_XEN),				\
		       __early_init);
/**
 * x86_init_fn_early_init: call all early_init() callbacks
 *
 * This calls all early_init() callbacks on the x86_init_fns linker table.
 */
void x86_init_fn_early_init(void);

#endif /* __X86_INIT_TABLES_H */
