#ifndef _ASM_GENERIC_TABLES_H_
#define _ASM_GENERIC_TABLES_H_

#include <asm/sections.h>

#define SECTION_TYPE_TABLES	tbl

#define SECTION_TBL(section, name, level)				\
	SECTION_TYPE(section, SECTION_TYPE_TABLES, name, level)

#define SECTION_TBL_ALL(section)					\
	SECTION_TYPE_ALL(section,SECTION_TYPE_TABLES)

#ifndef push_section_tbl
#define push_section_tbl(section, name, level, flags)			\
	push_section_type(section, SECTION_TYPE_TABLES, name, level, flags)
#endif

#ifndef push_section_tbl_any
#define push_section_tbl_any(section, name, flags)			\
	push_section_type(section, SECTION_TYPE_TABLES, name,		\
			  SECTION_TYPE_RANGES, flags)
#endif

#ifdef __ASSEMBLER__

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

#endif /* __ASSEMBLER__ */

#endif /* _ASM_GENERIC_TABLES_H_ */
