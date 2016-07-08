#ifndef _ASM_GENERIC_RANGES_H_
#define _ASM_GENERIC_RANGES_H_
/*
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */
#include <asm/sections.h>

#define SECTION_TYPE_RANGES	rng

#define SECTION_RANGE(section, name, level)				\
	SECTION_TYPE(section, SECTION_TYPE_RANGES, name, level)

#define SECTION_RANGE_ALL(section)					\
	SECTION_TYPE_ALL(section,SECTION_TYPE_RANGES)

#ifndef push_section_rng
#define push_section_rng(section, name, flags)				\
	push_section_type(section, SECTION_TYPE_RANGES, name,		\
			  SECTION_ORDER_ANY, flags)
#endif

#ifndef push_section_rng_level
#define push_section_rng_level(section, name, level, flags)			\
	push_section_type(section, SECTION_TYPE_RANGES, name, level, flags)
#endif

#ifdef __ASSEMBLER__

#ifndef DECLARE_SECTION_RANGE
#define DECLARE_SECTION_RANGE(section, name)				\
  push_section_rng(section, name,,) ;					\
  .globl name ;								\
name: ;									\
  .popsection								\
									\
  push_section_rng(section, name, ~,) ;					\
  .popsection
#endif

#endif /* __ASSEMBLER__ */

#endif /* _ASM_GENERIC_RANGES_H_ */
