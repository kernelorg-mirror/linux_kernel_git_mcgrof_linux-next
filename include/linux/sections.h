#ifndef _LINUX_SECTIONS_H
#define _LINUX_SECTIONS_H
/*
 * Linux de-facto sections
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

#include <asm/section-core.h>
#include <linux/export.h>

#ifndef __ASSEMBLY__

/**
 * DOC: Introduction
 *
 * Linux defines a set of common helpers which can be used to against its use
 * of standard or custom Linux sections, this section is dedicated to these
 * helpers.
 */

/**
 * LINUX_SECTION_ALIGNMENT - get section alignment
 *
 * @name: section name
 *
 * Gives you the alignment for the section.
 */
#define LINUX_SECTION_ALIGNMENT(name)	__alignof__(*VMLINUX_SYMBOL(name))

/**
 * LINUX_SECTION_SIZE - get number of entries in the section
 *
 * @name: section name
 *
 * This gives you the number of entries in the section.
 * Example usage:
 *
 *   unsigned int num_frobs = LINUX_SECTION_SIZE(frobnicator_fns);
 */
#define LINUX_SECTION_SIZE(name)					\
	((VMLINUX_SYMBOL(name##__end)) - (VMLINUX_SYMBOL(name)))

/**
 * LINUX_SECTION_EMPTY - check if section has no entries
 *
 * @name: section name
 *
 * Returns true if section is emtpy.
 *
 *   bool is_empty = LINUX_SECTION_EMPTY(frobnicator_fns);
 */
#define LINUX_SECTION_EMPTY(name)	(LINUX_SECTION_SIZE(name) == 0)

/**
 * LINUX_SECTION_START - get address of start of section
 *
 * @name: section name
 *
 * This gives you the start address of the section.
 * This should give you the address of the first entry.
 *
 */
#define LINUX_SECTION_START(name)	VMLINUX_SYMBOL(name)

/**
 * LINUX_SECTION_END - get address of end of the section
 *
 * @name: section name
 *
 * This gives you the end address of the section.
 * This should give you the address of the end of the
 * section. This will match the start address if the
 * section is empty.
 */
#define LINUX_SECTION_END(name)	VMLINUX_SYMBOL(name##__end)

/**
 * DECLARE_LINUX_SECTION - Declares a custom Linux section
 *
 * @type: type of custom Linux section
 * @name: custom section name
 *
 * Declares a read-write custom Linux section
 */
#define DECLARE_LINUX_SECTION(type, name)				\
	 extern type VMLINUX_SYMBOL(name)[], \
		     VMLINUX_SYMBOL(name##__end)[]

/**
 * DECLARE_LINUX_SECTION_RO - Declares a read-only custom Linux section
 *
 * @type: type of custom Linux section
 * @name: custom section name
 *
 * Declares a read-only custom Linux section
 */
#define DECLARE_LINUX_SECTION_RO(type, name)				\
	 extern const type VMLINUX_SYMBOL(name)[],			\
			   VMLINUX_SYMBOL(name##__end)[]

#define __SECTION_TYPE(section, type, name, level)			\
	#section "." #type "." #name "." #level

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_SECTIONS_H */
