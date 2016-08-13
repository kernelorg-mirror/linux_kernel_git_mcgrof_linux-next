#ifndef _LINUX_RANGES_H
#define _LINUX_RANGES_H
/*
 * Linux section ranges
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */
#include <linux/sections.h>
#include <asm/ranges.h>

#ifndef __ASSEMBLY__

/**
 * DOC: Introduction
 *
 * A section ranges consists of explicitly annotated series executable code
 * stitched together for the purpose of selective placement into standard or
 * architecture specific ELF sections. What ELF section is used is utility
 * specific. Linux has historically implicitly used section ranges, however
 * they were all built in an adhoc manner and typically required linker script
 * modifications per architecture. The section range API allows adding new
 * bundles of stiched executable code into custom ELF sections by only
 * modifying C or asm code in an architecture agnostic form.
 *
 * This documents the set of helpers available to declare, and define section
 * ranges and associate each section range to a specific Linux ELF section.
 */

/**
 * DOC: Section range module support
 *
 * Modules can use section ranges, however the section range definition must be
 * built-in to the kernel. That is, the code that implements
 * DEFINE_SECTION_RANGE() must be built-in, and modular code cannot add more
 * items in to the section range (with __LINUX_RANGE() or
 * __LINUX_RANGE_ORDER()), unless kernel/module.c find_module_sections() and
 * module-common.lds.S are updated accordingly with a respective module
 * notifier to account for updates. This restriction may be enhanced in the
 * future.
 */

/**
 * DOC: Section range helpers
 *
 * These are helpers for section ranges.
 */

/**
 * DECLARE_SECTION_RANGE - Declares a section range
 *
 * @name: section range name
 *
 * Declares a section range to help code access the range. Typically if
 * a subsystems needs code to have direct access to the section range the
 * subsystem's header file would declare the section range. Care should be
 * taken to only declare the section range in a header file if access to it
 * is truly needed outside of the code defining it. You typically would
 * rather instead provide helpers which access the section range with special
 * code on behalf of the caller.
 */
#define DECLARE_SECTION_RANGE(name)					\
	DECLARE_LINUX_SECTION_RO(char, name)

/**
 * __SECTION_RANGE_BEGIN - Constructs the beginning of a section range
 *
 * @name: section range name
 * @__section: ELF section to place section range into
 *
 * Constructs the beginning of a section range. You will typically not need
 * to use this directly.
 */
#define __SECTION_RANGE_BEGIN(name, __section)				\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_RNG_LEVEL(__section, name,))))

/**
 * __SECTION_RANGE_END - Constructs the end of a section range
 *
 * @name: section range name
 * @__section: ELF section to place section range into
 *
 * Constructs the end of a section range. You will typically not need
 * to use this directly.
 */
#define __SECTION_RANGE_END(name, __section)				\
	const __typeof__(VMLINUX_SYMBOL(name)[0])			\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_RNG_LEVEL(__section, name, ~))))

/**
 * DEFINE_SECTION_RANGE - Defines a section range
 *
 * @name: section range name
 * @section: ELF section name to place section range into
 *
 * Defines a section range, used for executable code. Section ranges are
 * defined in the code that takes ownership and makes use of the section
 * range.
 */
#define DEFINE_SECTION_RANGE(name, section)				\
	DECLARE_LINUX_SECTION_RO(char, name);				\
	__SECTION_RANGE_BEGIN(name, section) VMLINUX_SYMBOL(name)[0] = {};\
	__SECTION_RANGE_END(name, section) VMLINUX_SYMBOL(name##__end)[0] = {}

/**
 * SECTION_ADDR_IN_RANGE - returns true if address is in range
 *
 * @name: section range name
 * @addr: address to query for
 *
 * Returns true if the address is in the section range.
 */
#define SECTION_ADDR_IN_RANGE(name, addr)				\
	 (addr >= (unsigned long) LINUX_SECTION_START(name) &&		\
	  addr <  (unsigned long) LINUX_SECTION_END(name))

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_RANGES_H */
