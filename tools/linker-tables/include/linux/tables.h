#ifndef _LINUX_LINKER_TABLES_H
#define _LINUX_LINKER_TABLES_H
/*
 * Linux linker tables
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */
#include <linux/export.h>
#include <linux/sections.h>
#include <asm/tables.h>

#ifndef __ASSEMBLY__

/**
 * DOC: Introduction
 *
 * A linker table is a data structure that is stitched together from items in
 * multiple object files for the purpose of selective placement into standard
 * or architecture specific ELF sections. What section is used is utility
 * specific. Linux has historically implicitly used linker tables, however they
 * were all built in an adhoc manner which requires linker script modifications
 * per architecture. The linker table API provides a general facility so that
 * data structures can be stitched together and placed into Linux ELF sections
 * by only changing C or asm code in an architecture agnostic form.
 *
 * Linker tables help you group together related data and code in an efficient
 * way. Linker tables can be used to help simplify init sequences, they
 * enable linker build time selective sorting (disabled options get ignored),
 * and can optionally also be used to help you avoid code bit-rot due to
 * overuse of #ifdef.
 */

/**
 * DOC: Linker table provenance and userspace testing
 *
 * The Linux implementation of linker tables was inspired by the iPXE linker
 * table's solution (iPXE commit 67a10ef000cb7 "[contrib] Add rom-o-matic to
 * contrib "[0]).  To see how this code evolved refer to the out of tree
 * userspace linker-table tree [1]. Linux has a similar userspace application
 * in tools/linker-tables/ to help more easily test adding new extensions.
 *
 * Contrary to iPXE's solution which strives to force compilation of
 * everything using linker tables, Linux's solution allows for developers to be
 * selective over where one wishes to force compilation, this then is just an
 * optional feature for the Linux linker table solution. The main advantages
 * of using linker-tables then are:
 *
 *  - Avoiding modifying architecture linker scripts
 *  - Simplifying initialization code
 *  - Avoiding the code bit-rot problem
 *
 * [0] git://git.ipxe.org/ipxe.git
 *
 * [1] https://git.kernel.org/cgit/linux/kernel/git/mcgrof/linker-tables.git/
 */

/**
 * DOC: Avoids modifying architecture linker scripts
 *
 * Linker tables enable you to avoid modifying architecture linker scripts
 * since it has its has extended each core Linux section with a respective
 * linker table entry in `include/asm-generic/vmlinux.lds.h`. When you add new
 * linker table entry you aggregate them `into` the existing linker table core
 * section.
 */

/**
 * DOC: How linker tables simplify initialization code
 *
 * Traditionally, we would implement features in C code as follows:
 *
 *  foo_init();
 *
 * You'd then have a foo.h which would have::
 *
 *  #ifndef CONFIG_FOO
 *  static inline void foo_init(void) { }
 *  #endif
 *
 * With linker tables this is no longer necessary as your init routines would
 * be implicit, you'd instead call:
 *
 *  call_init_fns();
 *
 * call_init_fns() would call all functions present in your init table and if
 * and only if foo.o gets linked in, then its initialisation function will be
 * called, whether you use `obj-$(CONFIG_FOO)` or `table-obj-$(CONFIG_FOO)`.
 *
 * The linker script takes care of assembling the tables for us. All of our
 * table sections have names of the format `SECTION_NAME.tbl.NAME.N`. Here
 * `SECTION_NAME` is one of the standard sections in::
 *
 *   include/asm-generic/section-core.h
 *
 * and `NAME` designates the specific use case for the linker table, the table.
 * `N` is a digit used to help sort entries in the section. `N=` (empty string)
 * is reserved for the symbol indicating `table start`, and `N=~` is reserved
 * for the symbol indicating `table end`. In order for the call_init_fns() to
 * work behind the scenes the custom linker script would need to define the
 * beginning of the table, the end of the table, and in between it should use
 * ``SORT()`` to give order to the section. Typically this would require custom
 * linker script modifications however since linker table are already defined
 * in ``include/asm-generic/vmlinux.lds.h`` as documented above each new linker
 * table definition added in C code folds into the respective core Linux
 * section linker table.
 *
 * This is also done to support all architectures.  All that is needed then is
 * to ensure a respective common linker table entry is added to the shared
 * ``include/asm-generic/vmlinux.lds.h``.  There should be a respective::
 *
 *  *(SORT(SECTION_TBL_ALL(SECTION_NAME)))
 *
 * entry for each type of supported section there. If your `SECTION_NAME`
 * is not yet supported, consider adding support for it.
 *
 * Linker tables support ordering entries, it does this using a digit which
 * is eventually added as a postfix to a section entry name, we refer to this
 * as the linker table ``order-level``. If order is not important to your
 * linker table entry you can use the special ``SECTION_ORDER_ANY``. After
 * ``order-level``, the next contributing factor to order is the order of the
 * code in the C file, and the order of the objects in the Makefile. Using an
 * ``order-level`` then should not really be needed in most cases, its use
 * however enables to compartamentalize code into tables where ordering through
 * C file or through the Makefile would otherwise be very difficult or if one
 * wanted to enable very specific initialization semantics.
 *
 * As an example, suppose that we want to create a "frobnicator"
 * feature framework, and allow for several independent modules to
 * provide frobnicating services. Then we would create a frob.h
 * header file containing e.g.::
 *
 *	struct frobnicator {
 *		const char *name;
 *		void (*frob) (void);
 *	};
 *
 *	DECLARE_LINKTABLE(struct frobnicator, frobnicator_fns);
 *
 * Any module providing frobnicating services would look something
 * like::
 *
 *	#include "frob.h"
 *
 *	static void my_frob(void) {
 *		... Do my frobnicating
 *	}
 *
 *	LINKTABLE_INIT_DATA(frobnicator_fns, all) my_frobnicator = {
 *		.name = "my_frob",
 *		.frob = my_frob,
 *	};
 *
 * The central frobnicator code, say in frob.c, would use the frobnicating
 * modules as follows::
 *
 *	#include "frob.h"
 *
 *	void frob_all(void) {
 *		struct frobnicator *f;
 *
 *		LINKTABLE_FOR_EACH(f, frobnicator_fns) {
 *			pr_info("Calling frobnicator %s\n", frob->name);
 *			f->frob();
 *		}
 *	}
 */


/**
 * DOC: The code bit-rot problem
 *
 * Overuse of C #ifdefs can be problematic for certain types of code.  Linux
 * provides a rich array of features, but all these features take up valuable
 * space in a kernel image. The traditional solution to this problem has been
 * for each feature to have its own Kconfig entry and for the respective code
 * to be wrapped around #ifdefs, allowing the feature to be compiled in only
 * if enabled in Kconfig.
 *
 * The problem with this is that over time it becomes very difficult and time
 * consuming to compile, let alone test all possible Kconfig configurations.
 * Code that is not typically used tends to suffer from bit-rot over time. It
 * can become difficult to predict which combinations of compile-time options
 * will result in code that can compile and link correctly.
 */

/**
 * DOC: Avoiding the code bit-rot problem when desirable
 *
 * Linker tables can be used as one way to help solve the code bit-rot problem,
 * and in turn diminish Kconfig complexity.  To use linker tables and to
 * optionally take advantage of avoiding code bit-rot, feature code should be
 * implemented in separate C files, and should be designed to always be
 * compiled -- they should not be guarded with C code ``#ifdef CONFIG_FOO``
 * statements, consideration must also be taken for sub-features which depend
 * on the main ``CONFIG_FOO`` option, as they will be disabled if they depend
 * on ``CONFIG_FOO`` and therefore not compiled.
 *
 * To take advantage of this feature enable ``CONFIG_BUILD_AVOID_BITROT``, and
 * use special targets for your code. Either ``table-obj-y`` or ``table-lib-y``
 * can be used for your code instead of ``obj-y`` and ``lib-y``, respectively.
 * Without ``CONFIG_BUILD_AVOID_BITROT`` enabled these targets will work just
 * as their respective ``obj-y`` and ``lib-y`` counters work.  When
 * ``CONFIG_BUILD_AVOID_BITROT`` is enabled the code with the special targets
 * will always compile, even if the respective Kconfig entry for the code in
 * question has been disabled, this code however will only be linked in to the
 * final kernel image if the Kconfig entry for the code was enabled.
 *
 * Currently only built-in features are supported, modular support is not
 * yet supported, however you can make use of sub-features for modules
 * if they are independent and can simply be linked into modules.
 *
 * Care should be taken to vet that the code using this feature may also work
 * without ``CONFIG_BUILD_AVOID_BITROT``, otherwise it must depend on
 * CONFIG_BUILD_AVOID_BITROT.
 */

/**
 * DOC: Using target table-obj-y and table-lib-y
 *
 * Let's assume we want to always force compilation of feature ``FOO`` in the
 * kernel but avoid linking it. When you enable the ``FOO`` feature via Kconfig
 * you'd end up with::
 *
 *	#define CONFIG_FOO 1
 *
 * You typically would then just use this in your Makefile to selectively
 * compile and link the feature::
 *
 *	obj-$(CONFIG_FOO) += foo.o
 *
 * You could instead optionally use the new linker table target object::
 *
 *	table-obj-$(CONFIG_FOO) += foo.o
 *
 * Alternatively, this would be the equivalent of listing::
 *
 *	extra += foo.o
 *	obj-$(CONFIG_FOO) += foo.o
 *
 * Both are mechanisms which can be used to take advantage of forcing
 * compilation with linker tables, however making use of::
 *
 *	table-obj-$(CONFIG_FOO)
 *
 * is encouraged as it helps with annotating linker tables clearly where
 * compilation is forced. The ``table-lib-y`` target is the equivalent for
 * ``lib-y`` targets.
 */

/**
 * DOC: Linker table module support
 *
 * Modules can use linker tables, however the linker table definition
 * must be built-in to the kernel. That is, the code that implements
 * ``DEFINE_LINKTABLE*()`` must be built-in, and modular code cannot add
 * more items in to the table, unless ``kernel/module.c`` find_module_sections()
 * and module-common.lds.S are updated accordingly with a respective
 * module notifier to account for updates. This restriction may be enhanced
 * in the future.
 */

/**
 * DOC: Opting out of forcing compilation
 *
 * If you want to opt-out of forcing compilation simply disable
 * ``CONFIG_BUILD_AVOID_BITROT``.  Alternatively if your kernel configuration
 * has it and you must have it enabled and you want to opt-out of forcing
 * compilation you would use the typical ``obj-$(CONFIG_FOO) += foo.o`` and
 * ``foo.o`` will only be compiled and linked in when ``CONFIG_FOO`` enabled.
 * Using both ``table-obj-$(CONFIG_FOO)`` and ``obj-($CONFIG_FOO)`` will result
 * with the feature on your binary only if you've enabled ``CONFIG_FOO``,
 * however using ``table-obj-$(CONFIG_FOO)`` will always force compilation if
 * ``CONFIG_BUILD_AVOID_BITROT`` has been enabled.
 */

/**
 * DOC: Linker table helpers
 *
 * These are helpers for linker tables.
 */

/**
 * LINKTABLE_ADDR_WITHIN - returns true if address is in range
 *
 * @tbl: linker table
 * @addr: address to query for
 *
 * Returns true if the address is part of the linker table.
 */
#define LINKTABLE_ADDR_WITHIN(tbl, addr)				\
	 (addr >= (unsigned long) LINUX_SECTION_START(tbl) &&		\
          addr < (unsigned long) LINUX_SECTION_END(tbl))

/**
 * DOC: Constructing linker tables
 
 * Linker tables constructors are used to build an entry into a linker table.
 * Linker table constructors exist for each type of supported section.
 *
 * You have weak and regular type of link table entry constructors.
 */

/**
 * DOC: Weak linker tables constructors
 *
 * The weak attribute is desirable if you want an entry you can replace at
 * link time. A very special use case for linker tables is the first entry.
 * A weak attribute is used for the first entry to ensure that this entry's
 * address matches the end address of the table when the linker table is
 * emtpy, but will also point to the first real entry of the table once not
 * empty. When the first entry is linked in, it takes place of the first entry.
 */

/**
 * LINKTABLE_WEAK - Constructs a weak linker table entry for data
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table for data.
 */
#define LINKTABLE_WEAK(name, level)					\
	      __typeof__(VMLINUX_SYMBOL(name)[0])					\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_DATA,		\
				     name, level))))

/**
 * LINKTABLE_TEXT_WEAK - Constructs a weak linker table entry for execution
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table for code execution. These will be
 * read-only.
 */
#define LINKTABLE_TEXT_WEAK(name, level)				\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_TEXT,		\
				     name, level))))

/**
 * LINKTABLE_RO_WEAK - Constructs a weak read-only linker table entry
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table which only requires read-only access.
 */
#define LINKTABLE_RO_WEAK(name, level)					\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_TBL_RO,	\
						 name, level))))

/**
 * LINKTABLE_INIT_WEAK - Constructs a weak linker table entry for init code
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table for execution. use at init.
 */
#define LINKTABLE_INIT_WEAK(name, level)				\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_INIT,		\
						 name, level))))

/**
 * LINKTABLE_INIT_DATA_WEAK - Constructs a weak linker table entry for initdata
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table for data during init.
 */
#define LINKTABLE_INIT_DATA_WEAK(name, level)				\
	      __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_INIT_DATA,	\
						 name, level))))

/**
 * DOC: Regular linker linker table constructors
 *
 * Regular constructors are expected to be used for valid linker table entries.
 * Valid uses of weak entries other than the beginning and is currently
 * untested but should in theory work.
 */

/**
 * LINKTABLE - Declares a data linker table entry
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a data linker table entry. These are read-write.
 */
#define LINKTABLE(name, level)						\
	      __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_DATA,		\
						 name, level))))

/**
 * LINKTABLE_TEXT - Declares a linker table entry for execution
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table to be used for execution.
 */
#define LINKTABLE_TEXT(name, level)					\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_TEXT,		\
						 name, level))))

/**
 * LINKTABLE_RO - Declares a read-only linker table entry.
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table which only requires read-only access. Contrary
 * to LINKTABLE_RO_WEAK() which uses SECTION_RODATA this helper uses the
 * section SECTION_TBL_RO here due to possible toolchains bug on some
 * architectures, for instance the c6x architicture stuffs non-weak data
 * into different sections other than the one intended.
 */
#define LINKTABLE_RO(name, level)					\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_TBL_RO,	\
						 name, level))))

/**
 * LINKTABLE_INIT - Declares a linker table entry to be used on init.
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table entry for execution use during init.
 */
#define LINKTABLE_INIT(name, level)					\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGN_FUNC),	\
			     section(SECTION_TBL(SECTION_INIT,		\
						 name, level))))

/**
 * LINKTABLE_INIT_DATA - Declares a linker table entry to be used on init data.
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table entry for data during init.
 */
#define LINKTABLE_INIT_DATA(name, level)				\
	      __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_TBL(SECTION_INIT_DATA,	\
						 name, level))))

/**
 * DOC: Declaring Linker tables
 *
 * Declarers are used to help code access the linker tables. Typically
 * header files for subsystems would declare the linker tables to enable
 * easy access to add new entries, and to iterate over the list of table.
 * There are only two declarers needed given that the section association
 * is done by the definition of the linker table using ``DEFINE_LINKTABLE*()``
 * helpers.
 */


/**
 * DECLARE_LINKTABLE - Declares a data linker table entry
 *
 * @type: data type
 * @name: table name
 *
 * Declares a data linker table entry.
 */
#define DECLARE_LINKTABLE(type, name)					\
	DECLARE_LINUX_SECTION(type, name)

/**
 * DECLARE_LINKTABLE_RO - Declares a read-only linker table entry
 *
 * @type: data type
 * @name: table name
 *
 * Declares a read-only linker table entry.
 */
#define DECLARE_LINKTABLE_RO(type, name)				\
	DECLARE_LINUX_SECTION_RO(type, name)

/**
 * DOC: Defining Linker tables
 *
 * Linker tables are defined in the code that takes ownership over
 * the linker table. This is typically done in the same code that is in
 * charge of iterating over the linker table as well.
 */

/**
 * DEFINE_LINKTABLE - Defines a linker table for data
 *
 * @type: data type
 * @name: table name
 *
 * Defines a linker table which used for data.
 */
#define DEFINE_LINKTABLE(type, name)					\
	DECLARE_LINKTABLE(type, name);					\
	LINKTABLE_WEAK(name,) VMLINUX_SYMBOL(name)[0] = {};		\
	LINKTABLE(name, ~) VMLINUX_SYMBOL(name##__end)[0] = {}

/**
 * DEFINE_LINKTABLE_TEXT - Declares linker table entry for exectuion
 *
 * @type: data type
 * @name: table name
 *
 * Declares a linker table entry for execution.
 */
#define DEFINE_LINKTABLE_TEXT(type, name)				\
	DECLARE_LINKTABLE_RO(type, name);				\
	LINKTABLE_TEXT_WEAK(name,) VMLINUX_SYMBOL(name)[0] = {};	\
	LINKTABLE_TEXT(name, ~) VMLINUX_SYMBOL(name##__end)[0] = {}

/**
 * DEFINE_LINKTABLE_RO - Defines a read-only linker table
 *
 * @type: data type
 * @name: table name
 *
 * Defines a linker table which we know only requires read-only access.
 */
#define DEFINE_LINKTABLE_RO(type, name)					\
	DECLARE_LINKTABLE_RO(type, name);				\
	LINKTABLE_RO_WEAK(name,) VMLINUX_SYMBOL(name)[0] = {};		\
	LINKTABLE_RO(name, ~) VMLINUX_SYMBOL(name##__end)[0] = {}

/**
 * DEFINE_LINKTABLE_INIT - Defines an init time linker table for execution
 *
 * @type: data type
 * @name: table name
 *
 * Defines a linker table. If you are adding a new type you should
 * enable ``CONFIG_DEBUG_SECTION_MISMATCH`` and ensure routines that make
 * use of the linker tables get a respective __ref tag.
 */
#define DEFINE_LINKTABLE_INIT(type, name)				\
	DECLARE_LINKTABLE(type, name);					\
	LINKTABLE_INIT_WEAK(name,) VMLINUX_SYMBOL(name)[0] = {};	\
	LINKTABLE_INIT(name, ~) VMLINUX_SYMBOL(name##__end)[0] = {}

/**
 * DEFINE_LINKTABLE_INIT_DATA - Defines an init time linker table for data
 *
 * @type: data type
 * @name: table name
 *
 * Defines a linker table for init data. If you are adding a new type you
 * should enable ``CONFIG_DEBUG_SECTION_MISMATCH`` and ensure routines that
 * make use of the linker tables get a respective __ref tag.
 */
#define DEFINE_LINKTABLE_INIT_DATA(type, name)				\
	DECLARE_LINKTABLE(type, name);					\
	LINKTABLE_INIT_DATA_WEAK(name,) VMLINUX_SYMBOL(name)[0] = {};	\
	LINKTABLE_INIT_DATA(name, ~) VMLINUX_SYMBOL(name##__end)[0] = {}

/**
 * DOC: Iterating over Linker tables
 *
 * To make use of the linker tables you want to be able to iterate over
 * them. This section documents the different iterators available.
 */

/**
 * LINKTABLE_FOR_EACH - iterate through all entries within a linker table
 *
 * @pointer: entry pointer
 * @tbl: linker table
 *
 * Example usage::
 *
 *   struct frobnicator *frob;
 *
 *   LINKTABLE_FOR_EACH(frob, frobnicator_fns) {
 *     ...
 *   }
 */

#define LINKTABLE_FOR_EACH(pointer, tbl)				\
	for (pointer = LINUX_SECTION_START(tbl);			\
	     pointer < LINUX_SECTION_END(tbl);				\
	     pointer++)

/**
 * LINKTABLE_RUN_ALL - iterate and run through all entries on a linker table
 *
 * @tbl: linker table
 * @func: structure name for the function name we want to call.
 * @args...: arguments to pass to func
 *
 * Example usage::
 *
 *   LINKTABLE_RUN_ALL(frobnicator_fns, some_run,);
 */
#define LINKTABLE_RUN_ALL(tbl, func, args...)				\
do {									\
	size_t i;							\
	for (i = 0; i < LINUX_SECTION_SIZE(tbl); i++)			\
		(VMLINUX_SYMBOL(tbl)[i]).func (args);			\
} while (0)

/**
 * LINKTABLE_RUN_ERR - run each linker table entry func and return error if any
 *
 * @tbl: linker table
 * @func: structure name for the function name we want to call.
 * @args...: arguments to pass to func
 *
 * Example usage::
 *
 *   unsigned int err = LINKTABLE_RUN_ERR(frobnicator_fns, some_run,);
 */
#define LINKTABLE_RUN_ERR(tbl, func, args...)				\
({									\
	size_t i;							\
	int err = 0;							\
	for (i = 0; !err && i < LINUX_SECTION_SIZE(tbl); i++)		\
		err = (VMLINUX_SYMBOL(tbl)[i]).func (args);		\
		err; \
})

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_LINKER_TABLES_H */
