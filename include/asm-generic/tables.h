#ifndef _ASM_GENERIC_TABLES_H_
#define _ASM_GENERIC_TABLES_H_
/*
 * Linux linker tables
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

#ifdef __KERNEL__
# include <asm/sections.h>
#endif /* __KERNEL__ */

#define SECTION_TYPE_TABLES	tbl

#define SECTION_TBL(section, name, level)				\
	SECTION_TYPE(section, SECTION_TYPE_TABLES, name, level)

#define SECTION_TBL_ALL(section)					\
	SECTION_TYPE_ALL(section,SECTION_TYPE_TABLES)

#ifndef section_tbl
#define section_tbl(section, name, level, flags)			\
	section_type(section, SECTION_TYPE_TABLES, name,		\
		     level, flags)
#endif

#ifndef section_tbl_any
#define section_tbl_any(section, name, flags)				\
	section_type(section, SECTION_TYPE_TABLES, name,		\
		     SECTION_ORDER_ANY, flags)
#endif

#ifndef section_tbl_asmtype
#define section_tbl_asmtype(section, name, level, flags, asmtype)	\
	section_type_asmtype(section, SECTION_TYPE_TABLES, name,	\
			     level, flags, asmtype)
#endif

#ifndef push_section_tbl
#define push_section_tbl(section, name, level, flags)			\
	push_section_type(section, SECTION_TYPE_TABLES, name,		\
			  level, flags)
#endif

#ifndef push_section_tbl_any
#define push_section_tbl_any(section, name, flags)			\
	push_section_type(section, SECTION_TYPE_TABLES, name,		\
			  SECTION_ORDER_ANY, flags)
#endif

#if defined(__ASSEMBLER__) || defined(__ASSEMBLY__)

#ifndef DECLARE_SECTION_TBL
#define DECLARE_SECTION_TBL(section, name)				\
  push_section_tbl(section, name,,) ;					\
  .globl name ;								\
name: ;									\
  .popsection								\
									\
  push_section_tbl(section, name, ~,) ;					\
  .popsection
#endif

#endif /* defined(__ASSEMBLER__) || defined(__ASSEMBLY__) */

#endif /* _ASM_GENERIC_TABLES_H_ */
