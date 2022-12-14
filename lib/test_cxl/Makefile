# SPDX-License-Identifier: GPL-2.0
ccflags-y := -I$(srctree)/drivers/cxl/

obj-$(CONFIG_CXL_DEBUG_MOCK) +=		core.o
obj-$(CONFIG_CXL_DEBUG_MOCK_ACPI) +=	acpi.o

obj-$(CONFIG_TEST_CXL) += cxl_test.o
obj-$(CONFIG_TEST_CXL) += cxl_mock.o
obj-$(CONFIG_TEST_CXL) += cxl_mock_mem.o

cxl_test-y := cxl.o
cxl_mock-y := mock.o
cxl_mock_mem-y := mem.o
