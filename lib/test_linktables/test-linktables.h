#include <linux/tables.h>

struct test_linktable {
	int expected;
	int write_test;
	int (*op)(int input_digit);
};

#define test_linktable_init_text(__level, __op)				\
	static LINKTABLE_INIT(test_fns_init_text, __level)		\
	__test_fn_init_text_##__op = {					\
		.expected = __level,					\
		.op = __op,						\
};

#define test_linktable_init_data(__level, __op)				\
	static LINKTABLE_INIT_DATA(test_fns_init_data, __level)		\
	__test_fn_init_data_##__op = {					\
		.expected = __level,					\
		.op = __op,						\
};

#define test_linktable(__level, __op)					\
	static LINKTABLE(test_fns_data, __level)			\
	__test_fn_data_##__op = {					\
		.expected = __level,					\
		.op = __op,						\
};

#define test_linktable_text(__level, __op)				\
	static LINKTABLE_TEXT(test_fns_text, __level)			\
	__test_fn_text_##__op = {					\
		.expected = __level,					\
		.op = __op,						\
};

#define test_linktable_rodata(__level, __op)				\
	static LINKTABLE_RO(test_fns_rodata, __level)			\
	__test_fn_rodata_##__op = {					\
		.expected = __level,					\
		.op = __op,						\
};

DECLARE_LINKTABLE_RO(struct test_linktable, test_fns_init_text);
DECLARE_LINKTABLE(struct test_linktable, test_fns_init_data);
DECLARE_LINKTABLE(struct test_linktable, test_fns_data);
DECLARE_LINKTABLE_RO(struct test_linktable, test_fns_text);
DECLARE_LINKTABLE_RO(struct test_linktable, test_fns_rodata);
