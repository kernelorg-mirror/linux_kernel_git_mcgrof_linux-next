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
 * DOC: Linker table provenance
 *
 * The Linux implementation of linker tables was inspired by the iPXE linker
 * table's solution (iPXE commit 67a10ef000cb7 "[contrib] Add rom-o-matic to
 * contrib "[0]).  To see how this code evolved refer to the out of tree
 * userspace linker-table tree [1].
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
 * called.
 *
 * The linker script takes care of assembling the tables for us. All of our
 * table sections have names of the format `SECTION_NAME..tbl.NAME.N`. Here
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
 *  *(SORT(.foo..tbl.*))
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
 *		linktable_for_each(f, frobnicator_fns) {
 *			pr_info("Calling frobnicator %s\n", frob->name);
 *			f->frob();
 *		}
 *	}
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
 * DOC: Linker table helpers
 *
 * These are helpers for linker tables.
 */

/**
 * LINKTABLE_START - get address of start of linker table
 *
 * @name: name of the linker table
 *
 * This gives you the start address of the linker table.
 * This should give you the address of the first entry.
 *
 */
#define LINKTABLE_START(name)	LINUX_SECTION_START(name)

/**
 * LINKTABLE_END - get address of end of the linker table
 *
 * @name: name of the linker table
 *
 * This gives you the end address of the linker table.
 * This will match the start address if the linker table
 * is empty.
 */
#define LINKTABLE_END(name)	LINUX_SECTION_END(name)

/**
 * LINKTABLE_SIZE - get number of entries in the linker table
 *
 * @name: name of the linker table
 *
 * This gives you the number of entries in the linker table.
 * Example usage:
 *
 *   unsigned int num_frobs = LINKTABLE_SIZE(frobnicator_fns);
 */
#define LINKTABLE_SIZE(name)					\
	((LINKTABLE_END(name)) - (LINKTABLE_START(name)))

/**
 * LINKTABLE_EMPTY - check if linker table has no entries
 *
 * @name: name of linker table
 *
 * Returns true if the linker table is emtpy.
 *
 *   bool is_empty = LINKTABLE_EMPTY(frobnicator_fns);
 */
#define LINKTABLE_EMPTY(name)	(LINKTABLE_SIZE(name) == 0)

/**
 * LINKTABLE_ADDR_WITHIN - returns true if address is in the linker table
 *
 * @name: name of the linker table
 * @addr: address to query for
 *
 * Returns true if the address is part of the linker table.
 */
#define LINKTABLE_ADDR_WITHIN(name, addr)				\
	 (addr >= (unsigned long) LINKTABLE_START(name) &&		\
	  addr < (unsigned long) LINKTABLE_END(name))

/**
 * LINKTABLE_ALIGNMENT - get the alignment of the linker table
 *
 * @name: name of linker table
 *
 * Gives you the alignment for the linker table.
 */
#define LINKTABLE_ALIGNMENT(name)	LINUX_SECTION_ALIGNMENT(name)

/**
 * DOC: Constructing linker tables
 *
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
	      __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".data..tbl." #name "." #level)))

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
	const __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".text..tbl." #name "." #level)))

/**
 * LINKTABLE_RO_WEAK - Constructs a weak read-only linker table entry
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table which only requires read-only access.
 */
#define LINKTABLE_RO_WEAK(name, level)					\
	const __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(SECTION_TBL_RO_STR "..tbl." #name "." #level)))

/**
 * LINKTABLE_INIT_WEAK - Constructs a weak linker table entry for init code
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table for execution. use at init.
 */
#define LINKTABLE_INIT_WEAK(name, level)				\
	const __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".init.text..tbl." #name "." #level)))

/**
 * LINKTABLE_INIT_DATA_WEAK - Constructs a weak linker table entry for initdata
 *
 * @name: linker table name
 * @level: order level
 *
 * Constructs a weak linker table for data during init.
 */
#define LINKTABLE_INIT_DATA_WEAK(name, level)				\
	      __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".init.data..tbl." #name "." #level)))

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
	      __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".data..tbl." #name "." #level)))

/**
 * LINKTABLE_TEXT - Declares a linker table entry for execution
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table to be used for execution.
 */
#define LINKTABLE_TEXT(name, level)					\
	const __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".text..tbl." #name "." #level)))

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
	const __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(SECTION_TBL_RO_STR "..tbl." #name "." #level)))

/**
 * LINKTABLE_INIT - Declares a linker table entry to be used on init.
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table entry for execution use during init.
 */
#define LINKTABLE_INIT(name, level)					\
	const __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".init.text..tbl." #name "." #level)))

/**
 * LINKTABLE_INIT_DATA - Declares a linker table entry to be used on init data.
 *
 * @name: linker table name
 * @level: order level
 *
 * Declares a linker table entry for data during init.
 */
#define LINKTABLE_INIT_DATA(name, level)				\
	      __typeof__(LINKTABLE_START(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINKTABLE_ALIGNMENT(name)),	\
			     section(".init.data..tbl." #name "." #level)))

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
	LINKTABLE_WEAK(name,) LINKTABLE_START(name)[0] = {};		\
	LINKTABLE(name, ~) LINKTABLE_END(name)[0] = {}

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
	LINKTABLE_TEXT_WEAK(name,) LINKTABLE_START(name)[0] = {};	\
	LINKTABLE_TEXT(name, ~) LINKTABLE_END(name)[0] = {}

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
	LINKTABLE_RO_WEAK(name,) LINKTABLE_START(name)[0] = {};		\
	LINKTABLE_RO(name, ~) LINKTABLE_END(name)[0] = {}

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
	DECLARE_LINKTABLE_RO(type, name);				\
	LINKTABLE_INIT_WEAK(name,) LINKTABLE_START(name)[0] = {};	\
	LINKTABLE_INIT(name, ~) LINKTABLE_END(name)[0] = {}

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
	LINKTABLE_INIT_DATA_WEAK(name,) LINKTABLE_START(name)[0] = {};	\
	LINKTABLE_INIT_DATA(name, ~) LINKTABLE_END(name)[0] = {}

/**
 * DOC: Iterating over Linker tables
 *
 * To make use of the linker tables you want to be able to iterate over
 * them. This section documents the different iterators available.
 */

/**
 * linktable_for_each - iterate through all entries within a linker table
 *
 * @pointer: entry pointer
 * @tbl: linker table
 *
 * Example usage::
 *
 *   struct frobnicator *frob;
 *
 *   linktable_for_each(frob, frobnicator_fns) {
 *     ...
 *   }
 */

#define linktable_for_each(pointer, tbl)				\
	for (pointer = LINKTABLE_START(tbl);				\
	     pointer < LINKTABLE_END(tbl);				\
	     pointer++)

/**
 * linktable_run_all - iterate and run through all entries on a linker table
 *
 * @tbl: linker table
 * @func: structure name for the function name we want to call.
 * @args...: arguments to pass to func
 *
 * Example usage::
 *
 *   linktable_run_all(frobnicator_fns, some_run,);
 */
#define linktable_run_all(tbl, func, args...)				\
do {									\
	size_t i;							\
	for (i = 0; i < LINKTABLE_SIZE(tbl); i++)			\
		(LINKTABLE_START(tbl)[i]).func(args);			\
} while (0)

/**
 * linktable_run_err - run each linker table entry func and return error if any
 *
 * @tbl: linker table
 * @func: structure name for the function name we want to call.
 * @args...: arguments to pass to func
 *
 * Example usage::
 *
 *   unsigned int err = linktable_run_err(frobnicator_fns, some_run,);
 */
#define linktable_run_err(tbl, func, args...)				\
({									\
	size_t i;							\
	int err = 0;							\
	for (i = 0; !err && i < LINKTABLE_SIZE(tbl); i++)		\
		err = (LINKTABLE_START(tbl)[i]).func(args);		\
	err;								\
})

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_LINKER_TABLES_H */
