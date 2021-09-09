# SPDX-License-Identifier: GPL-2.0
include $(srctree)/drivers/base/firmware_loader/builtin/lib.Makefile

microcode-builtin-y :=
microcode-builtin-$(CONFIG_MICROCODE_BUILTIN_AMD_10H_14H) += microcode_amd.bin
microcode-builtin-$(CONFIG_MICROCODE_BUILTIN_AMD_15H) += microcode_amd_fam15h.bin
microcode-builtin-$(CONFIG_MICROCODE_BUILTIN_AMD_16H) += microcode_amd_fam16h.bin
microcode-builtin-$(CONFIG_MICROCODE_BUILTIN_AMD_17H) += microcode_amd_fam17h.bin

microcode-builtin-y := $(addprefix $(CONFIG_MICROCODE_BUILTIN_AMD_DIR)/,$(microcode-builtin-y))

fwdir := $(subst $(quote),,$(CONFIG_FW_LOADER_BUILTIN_DIR))
fwdir := $(addprefix $(srctree)/,$(filter-out /%,$(fwdir)))$(filter /%,$(fwdir))

firmware  := $(addsuffix .gen.o, $(subst $(quote),,$(microcode-builtin-y)))
obj-y += $(firmware)
