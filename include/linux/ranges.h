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

#define SECTION_ADDR_IN_RANGE(name, addr)				\
	 (addr >= (unsigned long) LINUX_SECTION_START(name) &&		\
          addr <  (unsigned long) LINUX_SECTION_END(name))

#define DECLARE_SECTION_RANGE(name)					\
	DECLARE_LINUX_SECTION_RO(char, name)

#define SECTION_RANGE_BEGIN(name, __section)				\
	const __typeof__(name[0])					\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_RANGE(__section, name, ))))

#define SECTION_RANGE_END(name, __section)				\
	const __typeof__(name[0])					\
	      __attribute__((used,					\
			     __aligned__(LINUX_SECTION_ALIGNMENT(name)),\
			     section(SECTION_RANGE(__section, name, ~))))

#define DEFINE_SECTION_RANGE(name, section)				\
	DECLARE_LINUX_SECTION_RO(char, name);				\
	SECTION_RANGE_BEGIN(name, section) VMLINUX_SYMBOL(name)[0] = {};\
	SECTION_RANGE_END(name, section) VMLINUX_SYMBOL(name##__end)[0] = {};

#define __LINUX_RANGE(section, name)					\
	__attribute__((__aligned__(LINUX_SECTION_ALIGNMENT(name)),	\
		       __section__(SECTION_RANGE(section, name, SECTION_ORDER_ANY))))

#define __LINUX_RANGE_ORDER(section, name, level)			\
	__attribute__((__aligned__(LINUX_SECTION_ALIGNMENT(name)),	\
		       __section__(SECTION_RANGE(section, name, level))))

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_RANGES_H */
