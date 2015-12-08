#ifndef _ASM_GENERIC_RANGES_H_
#define _ASM_GENERIC_RANGES_H_
/*
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */
#include <asm/section-core.h>

#define SECTION_RNG(section, name)					\
	SECTION_CORE(section, rng, name,				\
		     SECTION_ORDER_ANY)

#define SECTION_RNG_LEVEL(section, name, level)				\
	SECTION_CORE(section, rng, name, level)

#define SECTION_RNG_ALL(section)					\
	SECTION_CORE_ALL(section,rng)

#ifndef set_section_rng
# define set_section_rng(section, name, flags)				\
	 set_section_core(section, rng, name,				\
			  SECTION_ORDER_ANY, flags)
#endif

#ifndef set_section_rng_type
# define set_section_rng_type(section, name, flags, type)		\
	 set_section_core_type(section, rng, name,			\
			       SECTION_ORDER_ANY, flags, type)
#endif

#ifndef set_section_rng_level
# define set_section_rng_level(section, name, level, flags)		\
	 set_section_core(section, rng, name, level, flags)
#endif

#ifndef push_section_rng
# define push_section_rng(section, name, flags)				\
	 push_section_core(section, rng, name,				\
			   SECTION_ORDER_ANY, flags)
#endif

#ifndef push_section_rng_level
# define push_section_rng_level(section, name, level, flags)		\
	 push_section_core(section, rng, name,				\
			   level, flags)
#endif

#ifndef __ASSEMBLY__
/**
 * __LINUX_RANGE - short hand association into a section range
 *
 * @section: ELF section name to place section range into
 * @name: section range name
 *
 * This helper can be used by subsystems to define their own subsystem
 * specific helpers to easily associate a piece of code being defined to a
 * section range.
 */
#define __LINUX_RANGE(section, name)					\
	__attribute__((__section__(SECTION_RNG(section, name))))

/**
 * __LINUX_RANGE_ORDER - short hand association into a section range of order
 *
 * @section: ELF section name to place section range into
 * @name: section range name
 * @level: order level, a number. The order level gets tucked into the
 *	section as a postfix string. Order levels are sorted using
 * 	binutils SORT(), the number is sorted as a string, as such be
 * 	sure to fill with zeroes any empty digits. For instance if you are
 * 	using 3 levels of digits for order levels, use 001 for the first entry,
 * 	0002 for the second, 999 for the last entry. You can use however many
 * 	digits you need.
 *
 * This helper can be used by subsystems to define their own subsystem specific
 * helpers to easily associate a piece of code being defined to a section range
 * with an associated specific order level. The order level provides the
 * ability for explicit user ordering of code. Sorting takes place at link
 * time, after compilation.
 */
#define __LINUX_RANGE_ORDER(section, name, level)			\
	__attribute__((__section__(SECTION_RNG_LEVEL(section, name, level))))

#endif /* __ASSEMBLY__ */

#ifdef __ASSEMBLER__

#ifndef DEFINE_SECTION_RANGE
#define DEFINE_SECTION_RANGE(section, name)				\
  push_section_rng_level(section, name,,) ;					\
  .globl name ;								\
name: ;									\
  .popsection								\
									\
  push_section_rng_level(section, name, ~,) ;					\
  .popsection
#endif
#endif /* __ASSEMBLER__ */

#endif /* _ASM_GENERIC_RANGES_H_ */
