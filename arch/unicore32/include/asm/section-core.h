#ifndef __UNICORE_SECTION_CORE_ASM_H__
#define __UNICORE_SECTION_CORE_ASM_H__
/*
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

/* Unicore32 has known to not work properly with the type set, so ignore it */

#define __set_section_core_type(___section, ___core, ___name,		\
				___level, ___flags, ___type)		\
	.section ___section.___core.___name.___level, ___flags

#include <asm-generic/section-core.h>

#endif /* __UNICORE_SECTION_CORE_ASM_H__ */
