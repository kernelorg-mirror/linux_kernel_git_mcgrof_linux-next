include ../scripts/Makefile.include

all:

include ../scripts/utilities.mak

MAKEFLAGS += --no-print-directory

unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC

ifeq ($(srctree),)
	srctree := $(patsubst %/,%,$(dir $(shell pwd)))
srctree := $(patsubst %/,%,$(dir $(srctree)))
#$(info Determined 'srctree' to be $(srctree))
endif

ifneq ($(objtree),)
#$(info Determined 'objtree' to be $(objtree))
endif

ifneq ($(OUTPUT),)
#$(info Determined 'OUTPUT' to be $(OUTPUT))
# Adding $(OUTPUT) as a directory to look for source files,
# because use generated output files as sources dependency
# for flex/bison parsers.
VPATH += $(OUTPUT)
export VPATH
endif

export srctree OUTPUT RM CC LD AR CFLAGS V BISON FLEX AWK
include $(srctree)/tools/build/Makefile.include

ifeq ($(V),1)
	Q=
	NQ=@true
else
	Q=@
	NQ=@echo
endif

MAKEFLAGS += -r

define allow-override
  $(if $(or $(findstring environment,$(origin $(1))),\
            $(findstring command line,$(origin $(1)))),,\
    $(eval $(1) = $(2)))
endef

# Allow setting CC and AR and LD, or setting CROSS_COMPILE as a prefix.
$(call allow-override,CC,$(CROSS_COMPILE)gcc)
$(call allow-override,AR,$(CROSS_COMPILE)ar)
$(call allow-override,LD,$(CROSS_COMPILE)ld)

LD += $(EXTRA_LDFLAGS)

include $(srctree)/tools/scripts/Makefile.arch

# Refer to README to learn how to support your architecture
SUPPORTED_ARCHS = x86

ifeq ($(filter $(ARCH),$(SUPPORTED_ARCHS)),)
$(error Unsupported arch: $(ARCH))
endif

CFLAGS += -O2 -g
CFLAGS += -std=gnu99 -Wall -Werror
CFLAGS += -lpthread
CFLAGS += -DCONFIG_KPROBES

# We confine the includes used below to those we know are safe
# for this type of hack.
CFLAGS += -D__KERNEL__

# CFLAGS += -DCONFIG_HAVE_ARCH_PS_CONST
INCLUDES = \
	   -I include/ \
	   -I arch/$(ARCH)/include/ \
	   -I ../include/ \
	   -I ../arch/$(ARCH)/include/ \
	   -I ../arch/$(ARCH)/include/generated/

CFLAGS += $(INCLUDES)
CFLAGS +=-Wl,-Tarch/$(ARCH)/kernel/vmlinux.lds

HEADERS = \
	  ../../include/asm-generic/section-core.h \
	  ../../include/asm-generic/ranges.h \
	  ../../include/asm-generic/tables.h \
	  ../../include/linux/sections.h \
	  ../../include/linux/ranges.h \
	  ../../include/linux/tables.h

__check_headers: $(HEADERS)
	@$(foreach h, $(HEADERS), \
		(test -f $(h) && ( \
			(diff -B $(subst ../,../,$(h)) $(h) >/dev/null) \
			|| echo "Warning: $(subst ../../,tools/,$(h)) differs from kernel" >&2 ) || true);)

$(OUTPUT)arch/$(ARCH)/kernel/vmlinux.lds: arch/$(ARCH)/kernel/vmlinux.lds.S
	$(NQ) '  LD  ' $@
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -E -P \
		-D__ASSEMBLY__ -DLINKER_SCRIPT -o $@ $<

# Note, we use obj-y as a convenience factor, the cmd build
# stuff doesn't allow this magic, it resolves our depds but
# we still need to provide a series of targets for our objects.
# We stick to the convenient shorthand we're used to in the
# kernel. The final object resolution build stuff is handled
# by tool_target_obj and its caller below. This also enables
# OUTPUT=foo-path support so you can build externally.
obj-y += \
	arch/$(ARCH)/kernel/head64.o \
	arch/$(ARCH)/mm/init.o \
	kernel/locking/mutex.o \
	kernel/locking/spinlock.o \
	kernel/workqueue.o \
	kernel/main.o \
	pci.o \
	lib/string.o \
	pci-quirks.o \
	drivers/acme.o \
	drivers/synth/main.o \
	drivers/synth/common.o \
	drivers/synth/or.s \
	main.o

ifeq ($(ARCH),x86)
obj-y += \
	arch/$(ARCH)/kernel/alternative.o \
	arch/$(ARCH)/kernel/init.o \
	arch/$(ARCH)/kernel/kprobes.o \
	arch/$(ARCH)/kernel/kasan.o \
	arch/$(ARCH)/kernel/beta.o \
	arch/$(ARCH)/kernel/alpha.o \
	arch/$(ARCH)/xen/init.o	\
	drivers/xen-driver.o
endif

obj-y-out = $(patsubst %,$(OUTPUT)%,$(obj-y))

__build-dir = $(subst $(OUTPUT),,$(dir $1))
build-dir   = $(if $(__build-dir),$(__build-dir),.)

build := -f $(srctree)/tools/build/Makefile.build dir=$(build-dir)

define tool_target_obj
$(1): $(subst .s,.S,$(subst .o,.c,$(patsubst $(OUTPUT)%,%,$(1))))
	$(Q)$(MAKE) $(build) obj=$(1)
endef

$(foreach tool_obj, $(obj-y-out), \
	$(eval $(call tool_target_obj, $(tool_obj))))

$(OUTPUT)demo: $(obj-y-out)
	$(NQ) '  CC  ' $@
	$(Q)$(CC) $(obj-y-out) -o $@ $(CPPFLAGS) $(CFLAGS)

CHECK_HEADERS := __check_headers
PHONY += $(CHECK_HEADERS)

CMD_TARGETS = $(OUTPUT)arch/$(ARCH)/kernel/vmlinux.lds $(OUTPUT)demo
TARGETS = $(CMD_TARGETS)

all: $(CHECK_HEADERS) all_cmd

all_cmd: $(CMD_TARGETS)

ifeq ($(OUTPUT),)
clean_dir = ./
else
clean_dir = $(OUTPUT)
endif

clean::
	$(NQ)  '  CLEAN deps'
	$(Q)find $(clean_dir) -name \*.o \
		-o -name \*.s \
		-o -name \*.o.d \
		-o -name \*.o.cmd | xargs rm -f
	$(NQ)  '  CLEAN targets'
	$(Q)$(foreach f, $(CMD_TARGETS), rm -f $(f);)
