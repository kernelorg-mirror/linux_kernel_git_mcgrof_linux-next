// SPDX-License-Identifier: GPL-2.0

/*
 * Shared request-based / make_request-based functionality
 */
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>

struct dentry *blk_debugfs_root;

void blk_debugfs_register(void)
{
	blk_debugfs_root = debugfs_create_dir("block", NULL);
}
