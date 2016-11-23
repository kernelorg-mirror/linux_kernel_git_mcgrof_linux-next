__LINKTABLE_TESTS := $(shell find ./ -name test-linktables-\*.c | wc -l)
ccflags-y += -D__LINKTABLE_TESTS=$(__LINKTABLE_TESTS)

# Keep these disorganized, if linker tables did not work then
# things would not be ordered for us automatically at link time.
# Linker tables grants us the ability to be explicit about
# order.
obj-$(CONFIG_TEST_LINKTABLES) += test-linktables-04.o
obj-$(CONFIG_TEST_LINKTABLES) += test-linktables-02.o
obj-$(CONFIG_TEST_LINKTABLES) += test-linktables-01.o
obj-$(CONFIG_TEST_LINKTABLES) += test-linktables-03.o
obj-$(CONFIG_TEST_LINKTABLES) += test-linktables-00.o
obj-$(CONFIG_TEST_LINKTABLES) += test-linktables.o
