# SPDX-License-Identifier: GPL-2.0
extra-y := test-builtin-firmware.bin
$(obj)/test-builtin-firmware.bin:
	@$(kecho) "  GEN     $@"
	@(set -e;			\
	(				\
		echo 'ABCD0123';	\
	) > $@)
