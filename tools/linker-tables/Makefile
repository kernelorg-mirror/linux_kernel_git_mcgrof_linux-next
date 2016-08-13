include ../scripts/Makefile.include
include ../scripts/Makefile.arch

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS += -O2 -g
CFLAGS += -std=gnu99 -Wall -Werror
CFLAGS += -lpthread
CFLAGS += -DCONFIG_KPROBES

# We confine the includes used below to those we know are safe
# for this type of hack.
CFLAGS += -D__KERNEL__

# CFLAGS += -DCONFIG_HAVE_ARCH_PS_CONST
INCLUDES = -I include/ \
	   -I arch/$(ARCH)/include/ \
	   -I include/asm-generic/

CFLAGS += $(INCLUDES)
CFLAGS +=-Wl,-Tarch/$(ARCH)/kernel/vmlinux.lds

ifeq ($(V),1)
	Q=
	NQ=@true
else
	Q=@
	NQ=@echo
endif

HEADERS = \
	  ../../include/linux/stringify.h \
	  ../../include/linux/export.h \
	  ../../include/asm-generic/section-core.h \
	  ../../include/asm-generic/ranges.h \
	  ../../include/asm-generic/tables.h \
	  ../../include/linux/sections.h \
	  ../../include/linux/ranges.h \
	  ../../include/linux/tables.h

SETUP := __add_dirs __add_headers
PHONY += $(SETUP)

all: $(SETUP) arch/$(ARCH)/kernel/vmlinux.lds demo

__add_dirs:
	@mkdir -p include/linux/

__add_headers: $(HEADERS)
	@$(foreach h, $(HEADERS),\
		cp $(h) $(subst ../../,,$(h));)

arch/$(ARCH)/kernel/vmlinux.lds: arch/$(ARCH)/kernel/vmlinux.lds.S
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -E -P \
		-D__ASSEMBLY__ -DLINKER_SCRIPT -o $@ $<

%.s: %.S *.h
	$(NQ) '  AS  ' $@
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.c *.h
	$(QUIET_CC)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

OBJS =  \
	arch/$(ARCH)/kernel/head64.o \
	arch/$(ARCH)/mm/init.o \
	kernel/locking/mutex.o \
	kernel/locking/spinlock.o \
	kernel/workqueue.o \
	kernel/main.o \
	lib/string.o \
	pci.o \
	pci-quirks.o \
	drivers/acme.o \
	drivers/synth/main.o \
	drivers/synth/common.o \
	drivers/synth/or.S \
	demo.o

ifeq ($(ARCH),x86)
OBJS += \
	arch/$(ARCH)/kernel/alternative.o \
	arch/$(ARCH)/kernel/init.o \
	arch/$(ARCH)/kernel/kprobes.o \
	arch/$(ARCH)/kernel/kasan.o \
	arch/$(ARCH)/kernel/beta.o \
	arch/$(ARCH)/kernel/alpha.o \
	arch/$(ARCH)/xen/init.o	\
	drivers/xen-driver.o
endif

demo: $(OBJS)
	$(NQ) '  CC  ' $@
	$(Q)$(CC) $^ -o $@ $(CPPFLAGS) $(CFLAGS)

clean:
	find ./ -name \*.o | xargs rm -f
	@$(foreach h, $(HEADERS),\
		rm $(subst ../../,,$(h));)
	rm -f demo *.o arch/$(ARCH)/kernel/vmlinux.lds
