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
 * DOC: Introduction
 *
 * The Linux vmlinux binary uses a custom linker script which adds
 * some custom specialized ELF sections. This aims to document those
 * sections. Each section must document the goal of the section, and
 * address concurrency considerations when applicable.
 */

/**
 * DOC: Core Linux kernel sections
 *
 * These are the core Linux kernel sections.
 */

/**
 * SECTION_RODATA - read only data
 *
 * Macro name for code which must be protected from write access.
 */
#define SECTION_RODATA			.rodata

/**
 * SECTION_TEXT - kernel code execution section, read-only
 *
 * Macro name used to annotate code (functions) used during regular
 * kernel run time. This is combined with SECTION_RODATA, only this
 * section also gets execution allowed.
 *
 */
#define SECTION_TEXT			.text

/**
 * SECTION_DATA - for read-write data
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
 * SECTION_INIT - boot initialization code
 *
 * Macro name used to annotate code (functions) used only during boot or driver
 * initialization.
 *
 */
#define SECTION_INIT			.init.text

/**
 * SECTION_INIT_DATA - boot initialization data
 *
 * Macro name used to annotate data structures used only during boot or driver
 * initialization.
 */
#define SECTION_INIT_DATA		.init.data

/**
 * SECTION_INIT_RODATA - boot read-only initialization data
 *
 * Macro name used to annotate read-only code (functions) used only during boot
 * or driver initialization.
 */
#define SECTION_INIT_RODATA		.init.rodata

/**
 * SECTION_INIT_CALL - special init call
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
 * SECTION_EXIT - module exit code
 *
 * Macro name used to annotate code (functions) used only during module
 * unload.
 */
#define SECTION_EXIT			.exit.text

/**
 * SECTION_EXIT_DATA - module exit data structures
 *
 * Macro name used to annotate data structures used only during module
 * unload.
 */
#define SECTION_EXIT_DATA		.exit.data

/**
 * SECTION_EXIT_CALL - special exit call
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
 * sectioned code is valid. For more details refer to include/linux/init.h
 * __ref, __refdata, and __refconst documentation.
 */

/**
 * SECTION_REF - code referencing init is valid
 *
 * Macro name used to annotate that code (functions) declared with this section
 * has been vetteed as valid for its reference or use of other code (functions)
 * or data structures which are part of the init sections.
 */
#define SECTION_REF			.ref.text

/**
 * SECTION_REF_DATA - reference data structure are valid
 *
 * Macro name used to annotate data structures declared with this section have
 * been vetteed for its reference or use of other code (functions) or data
 * structures part of the init sections.
 */
#define SECTION_REF_DATA		.ref.data

/**
 * SECTION_REF_RODATA - const code or data structure referencing init is valid
 *
 * Macro name used to annotate const code (functions) const data structures
 * which has been vetteed for its reference or use of other code (functions)
 * or data structures part of the init sections.
 */
#define SECTION_REF_RODATA		.ref.rodata

#define SECTION_ORDER_ANY	any

/*
 * These section _ALL() helpers are for use on linker scripts and helpers
 */
#define SECTION_ALL(__section)						\
	__section##.*
#define SECTION_TYPE_ALL(__section, __type)				\
	__section##.##__type##.*

/* Can be used on foo.S for instance */
#ifndef __section_type_asmtype
# define __section_type_asmtype(__section, __type, __name,		\
				__level, __flags, __asm_type)		\
	.section __section.__type.__name.__level, __flags, __asm_type
#endif

#ifndef __section_type
# define __section_type(__section, __type, __name, __level, __flags)	\
	.section __section.__type.__name.__level, __flags
#endif

#ifndef __push_section_type
# define __push_section_type(__section, __type, __name, __level, __flags) \
	.pushsection __section.__type.__name.__level, __flags
#endif

#ifdef __KERNEL__
#include <linux/stringify.h>
#endif

#if defined(__ASSEMBLER__) || defined(__ASSEMBLY__)

# ifdef LINKER_SCRIPT

#  ifndef SECTION_TYPE
#   define SECTION_TYPE(__section, __type, __name, __level)		\
	__section.__type.__name.__level
#  endif

# else

#  ifndef push_section_type
#   define push_section_type(__section, __type, __name, __level, __flags) \
	 __push_section_type(__section, __type, __name,			  \
			     __level, __stringify(__flags))
#  endif

#  ifndef section_type
#   define section_type(__section, __type, __name, __level, __flags)	\
	__section_type(__section, __type, __name,			\
		       __level, __stringify(__flags))
#  endif

#  ifndef section_type_asmtype
#   define section_type_asmtype(__section, __type, __name,		\
				__level, __flags, __asmtype)		\
	__section_type_asmtype(__section, __type, __name, __level,	\
			       __stringify(__flags), __asmtype)
#  endif

#  ifndef SECTION_TYPE
#   define SECTION_TYPE(__section, __type, __name, __level)		\
	push_section_type(__section, __type, __name, __level,)
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

# ifndef section_type
#  define section_type(__section, __type, __name, __level, __flags)	\
	__stringify(__section_type(__section, __type, __name,		\
				   __level, __stringify(__flags)))	\
	ASM_CMD_SEP
# endif

/*
 * Some architectures (arm, and avr32 are two examples on kprobes) seem
 * currently explicitly specify the asm type [0] -- this can be any of
 * the optional constants on ELF:
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
# ifndef section_type_asmtype
#  define section_type_asmtype(__section, __type, __name, __level,	\
			     __flags, __asmtype)			\
	__stringify(__section_type_asmtype(__section, __type,		\
					   __name, __level,		\
					   __stringify(__flags),	\
					   asmtype))			\
	ASM_CMD_SEP
# endif

# ifndef push_section_type
#  define push_section_type(__section, __type, __name,			\
			    __level, __flags)				\
	__stringify(__push_section_type(__section, __type,		\
					__name,	__level,		\
					__stringify(__flags)))		\
	ASM_CMD_SEP
# endif

#endif /* defined(__ASSEMBLER__) || defined(__ASSEMBLY__) */
#endif /* _ASM_GENERIC_SECTION_CORE_H_ */
