#ifndef _LINUX_LINKER_TABLES_H
#define _LINUX_LINKER_TABLES_H

#define LINKTABLE_RO_WEAK(name, level)					\
	const __typeof__(name[0])					\
	      __attribute__((used,					\
			     weak,					\
			     __aligned__(__alignof__(__typeof__(name[0]))),\
			     section(".rodata.tbl." #name "." #level)))

#define LINKTABLE_RO(name, level)					\
	const __typeof__(name[0])					\
	      __attribute__((used,					\
			     __aligned__(__alignof__(__typeof__(name[0]))),\
			     section(".rodata.tbl." #name "." #level)))

#define DECLARE_LINKTABLE_RO(type, name)				\
	 extern const type name[], name##__end[];

#define DEFINE_LINKTABLE_RO(type, name)					\
	DECLARE_LINKTABLE_RO(type, name);				\
	LINKTABLE_RO_WEAK(name, ) VMLINUX_SYMBOL(name)[0] = {};		\
	LTO_REFERENCE_INITCALL(name);					\
	LINKTABLE_RO_WEAK(name, ~) VMLINUX_SYMBOL(name##__end)[0] = {};	\
	LTO_REFERENCE_INITCALL(name##__end);

#define LINKTABLE_SIZE(name)	((name##__end) - (name))

#define LINKTABLE_RUN_ALL(tbl, func, args...)				\
do {									\
	size_t i;							\
	for (i = 0; i < LINKTABLE_SIZE(tbl); i++)			\
		(tbl[i]).func (args);					\
} while (0);

#define LINKTABLE_RUN_ERR(tbl, func, args...)				\
({									\
	size_t i;							\
	int err = 0;							\
	for (i = 0; !err && i < LINKTABLE_SIZE(tbl); i++)		\
		err = (tbl[i]).func (args);				\
		err; \
})

#define LINKTABLE_FOR_EACH(pointer, tbl)				\
	for (pointer = tbl;						\
	     pointer < tbl##__end;					\
	     pointer++)

#endif /* _LINUX_LINKER_TABLES_H */
