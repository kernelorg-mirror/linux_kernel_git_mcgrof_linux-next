#ifndef _ASM_C6X_ASM_TABLES_H
#define _ASM_C6X_ASM_TABLES_H
/*
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

/*
 * The c6x toolchain has a bug present even on gcc-6 when non-weak attributes
 * are used and sends them to .rodata even though const data with weak
 * attributes are put in .const, this forces the linker to believe the address
 * is relative relative to the a base + offset and you end up with SB-relative
 * reloc error upon linking. Work around this by by forcing both start and
 * ending const RO waek linker table entry to be .const to fix this for now.
 *
 * [0] https://lkml.kernel.org/r/1470798247.3551.94.camel@redhat.com
 */

#define SECTION_TBL_RO		.const

#include <asm-generic/tables.h>

#endif /* _ASM_C6X_ASM_TABLES_H */
