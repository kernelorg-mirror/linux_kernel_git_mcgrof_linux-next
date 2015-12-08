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
#include <asm/section-core.h>

#define SECTION_TBL(section, name, level)	section..tbl.name.level

/* Some toolchains are buggy, let them override */
#ifndef SECTION_TBL_RO
# define SECTION_TBL_RO	.rodata
#endif

#define SECTION_TBL_RO_STR __stringify(SECTION_TBL_RO)

#ifndef set_section_tbl
# define set_section_tbl(section, name, level, flags)			\
	 set_section_core(section, tbl, name, level, flags)
#endif

#ifndef set_section_tbl_any
# define set_section_tbl_any(section, name, flags)			\
	 set_section_core(section, tbl, name, any, flags)
#endif

#ifndef set_section_tbl_type
# define set_section_tbl_type(section, name, level, flags, type)	\
	 set_section_core_type(section, tbl, name, level, flags, type)
#endif

#ifndef push_section_tbl
# define push_section_tbl(section, name, level, flags)			\
	 push_section_core(section, tbl, name, level, flags)
#endif

#ifndef push_section_tbl_any
# define push_section_tbl_any(section, name, flags)			\
	 push_section_core(section, tbl, name, any, flags)
#endif

#endif /* _ASM_GENERIC_TABLES_H_ */
