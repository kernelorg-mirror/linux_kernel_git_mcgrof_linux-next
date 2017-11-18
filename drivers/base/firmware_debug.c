// SPDX-License-Identifier: GPL-2.0
/* Firmware dubugging interface */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/debugfs.h>
#include "firmware_debug.h"

struct firmware_debug fw_debug;

static struct dentry *debugfs_firmware;

int __init register_fw_debugfs(void)
{
	debugfs_firmware = debugfs_create_dir("firmware", NULL);
	if (!debugfs_firmware)
		return -ENOMEM;

	if (!debugfs_create_bool("force_sysfs_fallback", S_IRUSR | S_IWUSR,
				 debugfs_firmware,
				 &fw_debug.force_sysfs_fallback))
		goto err_out;

	if (!debugfs_create_bool("ignore_sysfs_fallback", S_IRUSR | S_IWUSR,
				 debugfs_firmware,
				 &fw_debug.ignore_sysfs_fallback))
		goto err_out;

	return 0;
err_out:
	debugfs_remove_recursive(debugfs_firmware);
	debugfs_firmware = NULL;
	return -ENOMEM;
}

void unregister_fw_debugfs(void)
{
	debugfs_remove_recursive(debugfs_firmware);
	debugfs_firmware = NULL;
}
