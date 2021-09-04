// SPDX-License-Identifier: GPL-2.0
/* Builtin firmware support */

#include <linux/firmware.h>
#include "../firmware.h"

struct builtin_fw {
	char *name;
	void *data;
	unsigned long size;
};

extern struct builtin_fw __start_builtin_fw[];
extern struct builtin_fw __end_builtin_fw[];

static void fw_copy_to_prealloc_buf(struct firmware *fw,
				    void *buf, size_t size)
{
	if (!buf || size < fw->size)
		return;
	memcpy(buf, fw->data, fw->size);
}

/**
 * firmware_request_builtin() - load builtin firmware
 * @firmware_p: stack pointer to local firmware struct
 * @name: name of firmware file
 *
 * Some use cases in the kernel require looking for firmware which is built-in
 * to the kernel, and also have a requirement so that no memory allocator is
 * involved as these calls take place early in boot process. An example is the
 * x86 CPU microcode loader. In these cases all the caller wants is to see if
 * the firmware was built-in and if so use it right away.
 *
 * Callers of this API do not need to use release_firmware() as the pointer to
 * the firmware is expected to be provided locally on the stack of the caller.
 **/
bool firmware_request_builtin(struct firmware *fw, const char *name,
			      void *buf, size_t size)
{
	struct builtin_fw *b_fw;

	for (b_fw = __start_builtin_fw; b_fw != __end_builtin_fw; b_fw++) {
		if (strcmp(name, b_fw->name) == 0) {
			fw->size = b_fw->size;
			fw->data = b_fw->data;
			fw_copy_to_prealloc_buf(fw, buf, size);

			return true;
		}
	}

	return false;
}
EXPORT_SYMBOL_NS_GPL(firmware_request_builtin, TEST_FIRMWARE);

bool firmware_is_builtin(const struct firmware *fw)
{
	struct builtin_fw *b_fw;

	for (b_fw = __start_builtin_fw; b_fw != __end_builtin_fw; b_fw++)
		if (fw->data == b_fw->data)
			return true;

	return false;
}
