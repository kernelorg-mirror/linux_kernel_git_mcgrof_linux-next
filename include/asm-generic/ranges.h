#ifndef _ASM_GENERIC_RANGES_H_
#define _ASM_GENERIC_RANGES_H_
/*
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */
#ifdef __KERNEL__
# include <asm/sections.h>
#endif /* __KERNEL__ */

#define SECTION_TYPE_RANGES	rng

#define SECTION_RNG(section, name)					\
	SECTION_TYPE(section, SECTION_TYPE_RANGES, name,		\
		     SECTION_ORDER_ANY)

#define SECTION_RNG_LEVEL(section, name, level)				\
	SECTION_TYPE(section, SECTION_TYPE_RANGES, name, level)

#define SECTION_RANGE_ALL(section)					\
	SECTION_TYPE_ALL(section,SECTION_TYPE_RANGES)

#ifndef section_rng
#define section_rng(section, name, flags)				\
	section_type(section, SECTION_TYPE_RANGES, name,		\
		     SECTION_ORDER_ANY, flags)
#endif

#ifndef section_rng_asmtype
#define section_rng_asmtype(section, name, flags, asmtype)		\
	section_type_asmtype(section, SECTION_TYPE_RANGES, name,	\
			     SECTION_ORDER_ANY, flags, asmtype)
#endif

#ifndef section_rng_level
#define section_rng_level(section, name, level, flags)			\
	section_type(section, SECTION_TYPE_RANGES, name, level, flags)
#endif

#ifndef push_section_rng
#define push_section_rng(section, name, flags)				\
	push_section_type(section, SECTION_TYPE_RANGES, name,		\
			  SECTION_ORDER_ANY, flags)
#endif

#ifndef push_section_rng_level
#define push_section_rng_level(section, name, level, flags)		\
	push_section_type(section, SECTION_TYPE_RANGES, name,		\
			  level, flags)
#endif

#if defined(__ASSEMBLER__) || defined(__ASSEMBLY__)

#ifndef DECLARE_SECTION_RANGE
#define DECLARE_SECTION_RANGE(section, name)				\
  push_section_rng_level(section, name,,) ;				\
  .globl name ;								\
name: ;									\
  .popsection								\
									\
  push_section_rng_level(section, name, ~,) ;				\
  .popsection
#endif

#endif /* defined(__ASSEMBLER__) || defined(__ASSEMBLY__) */

#endif /* _ASM_GENERIC_RANGES_H_ */
