#ifndef _BLACKFIN_SECTION_CORE_H_
#define _BLACKFIN_SECTION_CORE_H_
/*
 * Linux section core definitions
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

#ifdef LINKER_SCRIPT
# define __FIX_SECTION_REF(__section)					\
	_##__section = __##__section;					\
	##__section = _##__section;					\
	_##__section##__end = __##__section##__end;			\
	##__section##__end = _##__section##__end
#endif /* LINKER_SCRIPT */

#include <asm-generic/section-core.h>

#endif /* _BLACKFIN_SECTION_CORE_H_ */
