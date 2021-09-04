// SPDX-License-Identifier: GPL-2.0
/* Builtin firmware support */

#include <linux/firmware.h>

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
