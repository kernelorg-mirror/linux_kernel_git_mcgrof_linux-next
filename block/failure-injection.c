// SPDX-License-Identifier: GPL-2.0

#include <linux/fault-inject.h>

#include "blk.h"

static DECLARE_FAULT_ATTR(fail_add_disk);
struct blk_config_add_disk_fail blk_config_add_disk_fail;

static int __init setup_fail_add_disk(char *str)
{
	return setup_fault_attr(&fail_add_disk, str);
}

__setup("fail_blk_add_disk=", setup_fail_add_disk);

struct dentry *config_fail_add_disk;

void blk_init_add_disk_fail(void)
{
	fault_create_debugfs_attr("fail_add_disk", blk_debugfs_root, &fail_add_disk);
	config_fail_add_disk = debugfs_create_dir("config_fail_add_disk", blk_debugfs_root);

	debugfs_create_bool("get_queue", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.get_queue);
	debugfs_create_bool("alloc_devt", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.alloc_devt);
	debugfs_create_bool("alloc_events", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.alloc_events);
	debugfs_create_bool("bdi_register", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.bdi_register);
	debugfs_create_bool("register_disk", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.register_disk);
	debugfs_create_bool("device_add", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.device_add);
	debugfs_create_bool("sysfs_depr_link", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.sysfs_depr_link);
	debugfs_create_bool("sysfs_bdi_link", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.sysfs_bdi_link);
	debugfs_create_bool("register_queue", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.register_queue);
	debugfs_create_bool("disk_add_events", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.disk_add_events);
	debugfs_create_bool("integrity_add", 0600, config_fail_add_disk,
			    &blk_config_add_disk_fail.integrity_add);
}

int __blk_should_fail_add_disk(bool evaluate)
{
	if (!evaluate)
		return 0;

	return should_fail(&fail_add_disk, 0);
}
