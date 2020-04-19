// SPDX-License-Identifier: GPL-2.0

/*
 * Shared request-based / make_request-based functionality
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>

struct dentry *blk_debugfs_root;

void blk_debugfs_register(void)
{
	blk_debugfs_root = debugfs_create_dir("block", NULL);
	if (!blk_debugfs_root)
		panic("Failed to create block debugfs directory\n");
}

int __must_check blk_queue_debugfs_register(struct request_queue *q)
{
	/* This can happen if we have a bug in the lower layers */
	BUG_ON(debugfs_lookup(kobject_name(q->kobj.parent), blk_debugfs_root));

	q->debugfs_dir = debugfs_create_dir(kobject_name(q->kobj.parent),
					    blk_debugfs_root);
	if (!q->debugfs_dir)
		return -ENOMEM;

	return 0;
}

void blk_queue_debugfs_unregister(struct request_queue *q)
{
	debugfs_remove_recursive(q->debugfs_dir);
	q->debugfs_dir = NULL;
}
