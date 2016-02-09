#ifndef _ASM_GENERIC_SECTIONS_H_
#define _ASM_GENERIC_SECTIONS_H_

#ifndef __ASSEMBLER__
#ifndef __ASSEMBLY__

/* References to section boundaries */

#include <linux/compiler.h>
#include <linux/types.h>

/*
 * Usage guidelines:
 * _text, _data: architecture specific, don't use them in arch-independent code
 * [_stext, _etext]: contains .text.* sections, may also contain .rodata.*
 *                   and/or .init.* sections
 * [_sdata, _edata]: contains .data.* sections, may also contain .rodata.*
 *                   and/or .init.* sections.
 * [__start_rodata, __end_rodata]: contains .rodata.* sections
 * [__init_begin, __init_end]: contains .init.* sections, but .init.text.*
 *                   may be out of this range on some architectures.
 * [_sinittext, _einittext]: contains .init.text.* sections
 * [__bss_start, __bss_stop]: contains BSS sections
 *
 * Following global variables are optional and may be unavailable on some
 * architectures and/or kernel configurations.
 *	_text, _data
 *	__kprobes_text_start, __kprobes_text_end
 *	__entry_text_start, __entry_text_end
 *	__ctors_start, __ctors_end
 */
extern char _text[], _stext[], _etext[];
extern char _data[], _sdata[], _edata[];
extern char __bss_start[], __bss_stop[];
extern char __init_begin[], __init_end[];
extern char _sinittext[], _einittext[];
extern char _end[];
extern char __per_cpu_load[], __per_cpu_start[], __per_cpu_end[];
extern char __kprobes_text_start[], __kprobes_text_end[];
extern char __entry_text_start[], __entry_text_end[];
extern char __start_rodata[], __end_rodata[];

/* Start and end of .ctors section - used for constructor calls. */
extern char __ctors_start[], __ctors_end[];

extern __visible const void __nosave_begin, __nosave_end;

/* function descriptor handling (if any).  Override
 * in asm/sections.h */
#ifndef dereference_function_descriptor
#define dereference_function_descriptor(p) (p)
#endif

/* random extra sections (if any).  Override
 * in asm/sections.h */
#ifndef arch_is_kernel_text
static inline int arch_is_kernel_text(unsigned long addr)
{
	return 0;
}
#endif

#ifndef arch_is_kernel_data
static inline int arch_is_kernel_data(unsigned long addr)
{
	return 0;
}
#endif

/**
 * memory_contains - checks if an object is contained within a memory region
 * @begin: virtual address of the beginning of the memory region
 * @end: virtual address of the end of the memory region
 * @virt: virtual address of the memory object
 * @size: size of the memory object
 *
 * Returns: true if the object specified by @virt and @size is entirely
 * contained within the memory region defined by @begin and @end, false
 * otherwise.
 */
static inline bool memory_contains(void *begin, void *end, void *virt,
				   size_t size)
{
	return virt >= begin && virt + size <= end;
}

/**
 * memory_intersects - checks if the region occupied by an object intersects
 *                     with another memory region
 * @begin: virtual address of the beginning of the memory regien
 * @end: virtual address of the end of the memory region
 * @virt: virtual address of the memory object
 * @size: size of the memory object
 *
 * Returns: true if an object's memory region, specified by @virt and @size,
 * intersects with the region specified by @begin and @end, false otherwise.
 */
static inline bool memory_intersects(void *begin, void *end, void *virt,
				     size_t size)
{
	void *vend = virt + size;

	return (virt >= begin && virt < end) || (vend >= begin && vend < end);
}

/**
 * init_section_contains - checks if an object is contained within the init
 *                         section
 * @virt: virtual address of the memory object
 * @size: size of the memory object
 *
 * Returns: true if the object specified by @virt and @size is entirely
 * contained within the init section, false otherwise.
 */
static inline bool init_section_contains(void *virt, size_t size)
{
	return memory_contains(__init_begin, __init_end, virt, size);
}

/**
 * init_section_intersects - checks if the region occupied by an object
 *                           intersects with the init section
 * @virt: virtual address of the memory object
 * @size: size of the memory object
 *
 * Returns: true if an object's memory region, specified by @virt and @size,
 * intersects with the init section, false otherwise.
 */
static inline bool init_section_intersects(void *virt, size_t size)
{
	return memory_intersects(__init_begin, __init_end, virt, size);
}

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

#endif /* __ASSEMBLY__ */
#endif /* __ASSEMBLER__ */

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
 * Macro name used to annotate const code (functions) const data structures which
 * has been vetteed for its reference or use of other code (functions) or data
 * structures part of the init sections.
 */
#define SECTION_REF_RODATA		.ref.rodata

#define SECTION_ORDER_ANY	any

/*
 * This section is for use on linker scripts and helpers
 */
#define SECTION_TYPE_ALL(section, type)					\
	section##.##type.*

#ifdef __ASSEMBLER__

/* Used on foo.S for instance */
#ifndef __push_section_type
#define __push_section_type(section, type, name, level, flags)		\
	.pushsection section.type.name.#level, #flags
#endif

#ifndef SECTION_TYPE
#define SECTION_TYPE(section, type, name, level)			\
	push_section_type(section, type, name, level, )
#endif

#else /* ! __ASSEMBLER__ */

/* Used in C code, for instance, on asm volatile() code. */

#ifndef __push_section_type
#define __push_section_type(section, type, name, level, flags)			\
	".pushsection " #section "." #type "." #name "." #level ",  \"" #flags "\"\n"
#endif

#endif /* ! __ASSEMBLER__ */

#ifndef push_section_type
#define push_section_type(section, type, name, level, flags)		\
	__push_section_type(section, type, name, level, flags)
#endif

#endif /* _ASM_GENERIC_SECTIONS_H_ */
