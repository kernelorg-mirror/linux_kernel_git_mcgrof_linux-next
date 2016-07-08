#ifndef _ASM_GENERIC_SECTION_CORE_H_
#define _ASM_GENERIC_SECTION_CORE_H_
/*
 * Linux section core definitions
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

/**
 * DOC: Linux ELF program specific sections
 *
 * Linux makes extensive use of ``SHT_PROGBITS`` to both extend use and
 * definition of *Special ELF Sections* (`gabi4 ch4`_) and to define its own
 * sections. This chapter is dedicated to documenting Linux program specific
 * sections and helpers available to make use of these easier to implement and
 * use.
 *
 * .. _gabi4 ch4: https://refspecs.linuxbase.org/elf/gabi4+/ch4.sheader.html#special_sections
 */

/**
 * DOC: Linux linker script
 *
 * Linux uses a custom linker script to build the vmlinux binary, it uses it to
 * strategically place and define Linux ELF sections.  Each architecture needs
 * to implement its own linker script, it is expected to reside in
 * ``arch/$(ARCH)/kernel/vmlinux.lds.S``.  Architecture Linux linker scripts in
 * turn include and use definitions from ``include/asm-generic/vmlinux.lds.h``,
 * as well as some helpers documented in this chapter.
 *
 * In assembly it is common practice to use dots (``.``) in labels to avoid
 * clashes with C symbols. Similarly, a dot (``.``) can be part of a section
 * name but not a C symbol. Historically then, two dots are used (``..``)
 * have been used in linker scripts when adding program specific sections
 * when there are concerns to avoid clashes with compiler generated sections.
 */

/**
 * DOC: Memory protection
 *
 * Linux allows architectures which support memory protection features to
 * take advantage of them by letting architectures define and enable
 * ``CONFIG_DEBUG_RODATA`` and implement a mark_rodata_ro() call.
 * mark_rodata_ro() can be used for instance to mark specific sections as
 * read-only or non-executable.
 *
 * Linux typically follows a convention to have the .rodata ELF section follow
 * after the .text ELF section, it does this to help architectures which
 * support memory protection to mark both .text and .rodata as read-only in
 * one shot.
 *
 * For more details refer to mark_rodata_ro().
 */

/**
 * DOC: .rodata
 *
 * ELF section used for data which must be protected from write access.
 */

/**
 * DOC: .text
 *
 * ELF section name used for code (functions) used during regular
 * kernel run time.
 */

/**
 * DOC: .data
 *
 * ELF section used for read-write data.
 */

/**
 * DOC: Linux init sections
 *
 * These sections are used for code and data structures used during boot or
 * module initialization. On architectures that support it (x86, x86_64), all
 * this code is freed up by the kernel right before the fist userspace init
 * process is called when built-in to the kernel, and if modular it is freed
 * after module initialization. Since the code is freed so early, in theory
 * there should be no races against freeing this code with other CPUs. Init
 * section code and data structures should never be exported with
 * EXPORT_SYMBOL*() as the code will quickly become unavailable to the kernel
 * after bootup.
 */

/**
 * DOC: .init.text
 *
 * ELF section for code (functions) used only during boot or driver
 * initialization.
 *
 */

/**
 * DOC: .init.data
 *
 * ELF section used for data structures used only during boot or driver
 * initialization.
 */

/**
 * DOC: .init.rodata
 *
 * ELF section used for read-only code (functions) used only during boot
 * or driver initialization.
 */

/**
 * DOC: .initcall
 *
 * ELF section used for subsystem init calls. There are init levels
 * representing different functionality in the kernel. For more details
 * refer to __define_initcall().
 */

/**
 * DOC: Linux exit sections
 *
 * These sections are used to declare a functions and data structures which
 * are only required on exit, the function or data structure will be dropped
 * if the code declaring this section is not compiled as a module on
 * architectures that support this (x86, x86_64). There is no special case
 * handling for this code when built-in to the kernel.
 */

/**
 * DOC: .exit.text
 *
 * ELF section used to for code (functions) used only during module unload.
 */

/**
 * DOC: .exit.data
 *
 * ELF section used to for data structures used only during module
 * unload.
 */

/**
 * DOC: .exitcall.exit
 *
 * ELF section used for exit routines, order is important and maintained by
 * link order.
 */

/**
 * DOC: Linux references to init sections
 *
 * These sections are used to teach modpost to not warn about possible
 * misuses of init section code from other sections. If you use this
 * your use case should document why you are certain such use of init
 * sectioned code is valid. For more details refer to ``include/linux/init.h``
 * ``__ref``, ``__refdata``, and ``__refconst`` documentation.
 */

/**
 * DOC: .ref.text
 *
 * ELF section used to annotate code (functions) which has been vetted as
 * valid for its reference or use of other code (functions) or data structures
 * which are part of the init sections.
 */

/**
 * DOC: .ref.data
 *
 * ELF section used for data structures which have been vetted for its
 * reference or use of other code (functions) or data structures part of the
 * init sections.
 */

/**
 * DOC: .ref.rodata
 *
 * ELF section used to annotate const code (functions) const data structures
 * which has been vetted for its reference or use of other code (functions)
 * or data structures part of the init sections.
 */

/**
 * DOC: Linux section ordering
 *
 * Linux may use binutils linker-script 'SORT()' on sections to sort Linux
 * sections alpha numerically. Linux has historically used 'SORT()' in
 * ``include/asm-generic/vmlinux.lds.h``, its a well established practice. If
 * 'SORT()' is used on a section one can provide ordering using a postfix on
 * each section entry added. For instance if a linker script uses::
 *
 *    SORT(.foo.*)
 *
 * one can then add entries with explicit ordering using numeric postfixes for
 * each entry, we refer to these as 'order levels'. Since 'SORT()' sorts alpha
 * numerically a specific series set of digits must be agreed a-priori which
 * would give also an idea of the max expected number of entries added to a
 * section. For instance, if you expect a maximum of 999 entries you can use
 * 3 digits for a section order level. If you wanted an entry to be ordered
 * first you could use the postfix '000', if you wanted an entry to follow this
 * you could use '001', and so on. We could for instance have::
 *
 *    .foo.000
 *    .foo.001
 *    .foo.002
 *
 * Often times one may want the option to specify no order is required for
 * certain elements added to a section which does use 'SORT()' on the linker
 * script. You can use any arbitrary string value to to specify no order is
 * used, so long as its used consistantly. For instance, one possibility is to
 * use the 'any' postfix.  All entries on the section would then have no
 * specific ordering::
 *
 *    .foo.any
 *    .foo.any
 *    .foo.any
 *
 * To help establish a convention we reserve the special name 'any' for this
 * purpose. Developers can use and expect the 'any' postfix string on sections
 * as a helper to annotate section ordering at link time is not relevant
 * for entries on a section.
 */

/* Can be used on foo.S for instance */
#ifndef __set_section_core_type
# define __set_section_core_type(___section, ___core, ___name,		\
				 ___level, ___flags, ___type)		\
	.section ___section..___core.___name.___level, ___flags, ___type
#endif

#ifndef __set_section_core
# define __set_section_core(___section, ___core, ___name, ___level, ___flags) \
	.section ___section..___core.___name.___level, ___flags
#endif

#ifndef __push_section_core
# define __push_section_core(__section, __core, __name, __level, __flags) \
	.pushsection __section..__core.__name.__level, __flags
#endif

#ifdef __KERNEL__
#include <linux/stringify.h>
#endif

#if defined(__ASSEMBLER__) || defined(__ASSEMBLY__)

# ifndef LINKER_SCRIPT

#  ifndef push_section_core
#   define push_section_core(__section, __core, __name, __level, __flags) \
	 __push_section_core(__section, __core, __name,			  \
			     __level, __stringify(__flags))
#  endif

#  ifndef set_section_core
#   define set_section_core(__section, __core, __name,			\
			    __level, __flags)				\
	__set_section_core(__section, __core, __name,			\
			   __level, __stringify(__flags))
#  endif

#  ifndef set_section_core_type
#   define set_section_core_type(__section, __core, __name,		\
				 __level, __flags, __type)		\
	__set_section_core_type(__section, __core, __name, __level,	\
				__stringify(__flags), __type)
#  endif

# endif /* LINKER_SCRIPT */
#else /* defined(__ASSEMBLER__) || defined(__ASSEMBLY__) */

/*
 * As per gcc's documentation a common asm separator is a new line followed
 * by tab [0], it however seems possible to also just use a newline as its
 * the most commonly empirically observed semantic and folks seem to agree
 * this even works on S390. In case your architecture disagrees you may
 * override this and define your own and keep the rest of the macros.
 *
 * [0] https://gcc.gnu.org/onlinedocs/gcc/Basic-Asm.html#Basic-Asm
 */
# ifndef ASM_CMD_SEP
#  define ASM_CMD_SEP	"\n"
# endif

# ifndef set_section_core
#  define set_section_core(__section, __core, __name, __level, __flags)	\
	__stringify(__set_section_core_type(__section, __core, __name,	\
					    __level, __stringify(__flags))) \
	ASM_CMD_SEP
# endif

/*
 * ARM currently requires to be explicitl about the type [0] -- this can be any
 * of the optional constants on ELF:
 *
 * @progbits - section contains data
 * @nobits - section does not contain data (i.e., section only occupies space)
 * @note - section contains data which is used by things other than the program
 * @init_array - section contains an array of pointers to init functions
 * @fini_array - section contains an array of pointers to finish functions
 * @preinit_array - section contains an array of pointers to pre-init functions
 *
 * ARM requires % instead of @.
 *
 * At least as per nasm (x86/x86_64 only), in the absence of qualifiers the
 * defaults are as follows:
 *
 * section .text    progbits  alloc   exec    nowrite  align=16
 * section .rodata  progbits  alloc   noexec  nowrite  align=4
 * section .lrodata progbits  alloc   noexec  nowrite  align=4
 * section .data    progbits  alloc   noexec  write    align=4
 * section .ldata   progbits  alloc   noexec  write    align=4
 * section .bss     nobits    alloc   noexec  write    align=4
 * section .lbss    nobits    alloc   noexec  write    align=4
 * section .tdata   progbits  alloc   noexec  write    align=4    tls
 * section .tbss    nobits    alloc   noexec  write    align=4    tls
 * section .comment progbits  noalloc noexec  nowrite  align=1
 * section other    progbits  alloc   noexec  nowrite  align=1
 *
 * gas should have sensible defaults for architectures...
 *
 * [0] http://www.nasm.us/doc/nasmdoc7.html
 */
# ifndef set_section_core_type
#  define set_section_core_type(__section, __core, __name, __level,	\
				__flags, __type)			\
	__stringify(__set_section_core_type(__section, __core,		\
					    __name, __level,		\
					    __stringify(__flags),	\
					    __type))			\
	ASM_CMD_SEP
# endif

# ifndef push_section_core
#  define push_section_core(__section, __core, __name,			\
			    __level, __flags)				\
	__stringify(__push_section_core(__section, __core,		\
					__name,	__level,		\
					__stringify(__flags)))		\
	ASM_CMD_SEP
# endif

#endif /* defined(__ASSEMBLER__) || defined(__ASSEMBLY__) */
#endif /* _ASM_GENERIC_SECTION_CORE_H_ */
