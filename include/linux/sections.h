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
#include <asm/sections.h>

#ifndef __ASSEMBLY__

/**
 * DOC: Linux section helpers
 *
 * Set of common helpers which can be used to against custom Linux sections.
 */

/**
 * LINUX_SECTION_ALIGNMENT - get section alignment
 *
 * @name: section name
 *
 * Gives you the alignment for the section.
 */
#define LINUX_SECTION_ALIGNMENT(name)	__alignof__(*name)

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
#define LINUX_SECTION_SIZE(name)	((name##__end) - (name))

/**
 * LINUX_SECTION_EMPTY - check if section has no entries
 *
 * @name: section name
 *
 * Returns true if section is emtpy.
 *
 *   bool is_empty = LINUX_SECTION_EMPTY(frobnicator_fns);
 */
#define LINUX_SECTION_EMPTY(name)	(LINUX_RANGE_SIZE(name) == 0)

/**
 * LINUX_SECTION_START - get address of start of section
 *
 * @name: section name
 *
 * This gives you the start address of the section.
 * This should give you the address of the first entry.
 *
 */
#define LINUX_SECTION_START(name)	name

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
#define LINUX_SECTION_END(name)	name##__end

/**
 * DECLARE_LINUX_SECTION - Declares a custom Linux section
 *
 * @type: type of custom Linux section
 * @name: custom section name
 *
 * Declares a read-write custom Linux section
 */
#define DECLARE_LINUX_SECTION(type, name)				\
	 extern type name[], name##__end[];

/**
 * DECLARE_LINUX_SECTION_RO - Declares a read-only custom Linux section
 *
 * @type: type of custom Linux section
 * @name: custom section name
 *
 * Declares a read-only custom Linux section
 */
#define DECLARE_LINUX_SECTION_RO(type, name)				\
	 extern const type name[], name##__end[];

#define __SECTION_TYPE(section, type, name, level)			\
	#section "." #type "." #name "." #level

/**
 * SECTION_TYPE - helper to construct a custom section type name
 *
 * @section: known de facto section, in asm-generic/sections.h
 * @type: type of Linux section
 * @name: custom section name
 * @level: order level, used to help sort the section entries. The value
 *	used should be a digit. Since ELF sections are strings there is
 *	no technical restriction on the length of the number of digits
 *	used.
 *
 * Helper to construct the name of a custom section type.
 */
#define SECTION_TYPE(section, type, name, level)                       	\
	__SECTION_TYPE(section, type, name, level)

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_SECTIONS_H */
