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
 * DOC: Custom linker script
 *
 * The Linux vmlinux binary uses a custom linker script on each architecture
 * which it uses to strategically place standard ELF sections and also adds
 * custom specialized ELF sections. Each architecture defines its own custom
 * linker defined in arch/$(ARCH)/kernel/vmlinux.lds.S -- these in turn
 * include and use definitions in include/asm-generic/vmlinux.lds.h as well
 * as some helpers documented in this chaper.
 */

/**
 * DOC: Standard ELF section use in Linux
 *
 * Linux makes use of the standard ELF sections, this sections documents
 * these.
 */

/**
 * DOC: SECTION_RODATA
 *
 * Macro name for code which must be protected from write access, read only
 * data.
 */
#define SECTION_RODATA			.rodata

/**
 * DOC: SECTION_TEXT
 *
 * Macro name used to annotate code (functions) used during regular
 * kernel run time. This is combined with `SECTION_RODATA`, only this
 * section also allows for execution.
 *
 */
#define SECTION_TEXT			.text

/**
 * DOC: SECTION_DATA
 *
 * Macro name for read-write data.
 */
#define SECTION_DATA			.data

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
 * DOC: SECTION_INIT
 *
 * Macro name used to annotate code (functions) used only during boot or driver
 * initialization.
 *
 */
#define SECTION_INIT			.init.text

/**
 * DOC: SECTION_INIT_DATA
 *
 * Macro name used to annotate data structures used only during boot or driver
 * initialization.
 */
#define SECTION_INIT_DATA		.init.data

/**
 * DOC: SECTION_INIT_RODATA
 *
 * Macro name used to annotate read-only code (functions) used only during boot
 * or driver initialization.
 */
#define SECTION_INIT_RODATA		.init.rodata

/**
 * DOC: SECTION_INIT_CALL
 *
 * Special macro name used to annotate subsystem init call. These calls are
 * are now grouped by functionality into separate subsections. Ordering inside
 * the subsections is determined by link order.
 */
#define SECTION_INIT_CALL		.initcall

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
 * DOC: SECTION_EXIT
 *
 * Macro name used to annotate code (functions) used only during module
 * unload.
 */
#define SECTION_EXIT			.exit.text

/**
 * DOC: SECTION_EXIT_DATA
 *
 * Macro name used to annotate data structures used only during module
 * unload.
 */
#define SECTION_EXIT_DATA		.exit.data

/**
 * DOC: SECTION_EXIT_CALL
 *
 * Special macro name used to annotate an exit exit routine, order
 * is important and maintained by link order.
 */
#define SECTION_EXIT_CALL		.exitcall.exit

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
 * DOC: SECTION_REF
 *
 * Macro name used to annotate that code (functions) declared with this section
 * has been vetteed as valid for its reference or use of other code (functions)
 * or data structures which are part of the init sections.
 */
#define SECTION_REF			.ref.text

/**
 * DOC: SECTION_REF_DATA
 *
 * Macro name used to annotate data structures declared with this section have
 * been vetteed for its reference or use of other code (functions) or data
 * structures part of the init sections.
 */
#define SECTION_REF_DATA		.ref.data

/**
 * DOC: SECTION_REF_RODATA
 *
 * Macro name used to annotate const code (functions) const data structures
 * which has been vetteed for its reference or use of other code (functions)
 * or data structures part of the init sections.
 */
#define SECTION_REF_RODATA		.ref.rodata

/**
 * DOC: Linux section ordering
 *
 * Linux may use binutils linker-script 'SORT()' on sections to sort Linux
 * sections. Linux has used 'SORT()' in ``include/asm-generic/vmlinux.lds.h``
 * for years.
 */

/**
 * DOC: SECTION_ORDER_ANY
 *
 * Macro name which can be used as helper to annotate custom section
 * ordering at link time is not relevant for specific sections.
 */
#define SECTION_ORDER_ANY	any

/*
 * These section _ALL() helpers are for use on linker scripts and helpers
 */
#define SECTION_ALL(__section)						\
	__section##.*

#define __SECTION_CORE(__section, __core, __name, __level)		\
	__section.__core.__name.__level

#define SECTION_CORE_ALL(__section, __core)				\
	__section##.##__core##.*

/* Can be used on foo.S for instance */
#ifndef __set_section_core_type
# define __set_section_core_type(___section, ___core, ___name,		\
				 ___level, ___flags, ___type)		\
	.section ___section.___core.___name.___level, ___flags, ___type
#endif

#ifndef __set_section_core
# define __set_section_core(___section, ___core, ___name, ___level, ___flags) \
	.section ___section.___core.___name.___level, ___flags
#endif

#ifndef __push_section_core
# define __push_section_core(__section, __core, __name, __level, __flags) \
	.pushsection __section.__core.__name.__level, __flags
#endif

#ifdef __KERNEL__
#include <linux/stringify.h>
#endif

#if defined(__ASSEMBLER__) || defined(__ASSEMBLY__)

# ifdef LINKER_SCRIPT

#  ifndef SECTION_CORE
#   define SECTION_CORE(__section, __core, __name, __level)		\
	__SECTION_CORE(__section,__core,__name,__level)
#  endif

# else

#  ifndef SECTION_CORE
#   define SECTION_CORE(__section, __core, __name, __level)		\
	push_section_core(__section, __core, __name, __level,)
#  endif

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

# ifndef SECTION_CORE
#  define SECTION_CORE(__section, __core, __name, __level)		\
	__stringify(__SECTION_CORE(__section,__core,__name,__level))
# endif

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
 * Some architectures (arm, and avr32 are two examples on kprobes) seem
 * currently explicitly specify the type [0] -- this can be any of the
 * optional constants on ELF:
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
