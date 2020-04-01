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

static struct dentry *blk_debugfs_dir_register(const char *name)
{
	return debugfs_create_dir(name, blk_debugfs_root);
}

void blk_queue_debugfs_register(struct request_queue *q, const char *name)
{
	q->debugfs_dir = blk_debugfs_dir_register(name);
}
EXPORT_SYMBOL_GPL(blk_queue_debugfs_register);

void blk_queue_debugfs_unregister(struct request_queue *q)
{
	debugfs_remove_recursive(q->debugfs_dir);
	q->debugfs_dir = NULL;
}
EXPORT_SYMBOL_GPL(blk_queue_debugfs_unregister);

void blk_part_debugfs_register(struct hd_struct *p, const char *name)
{
	p->debugfs_dir = blk_debugfs_dir_register(name);
}

void blk_part_debugfs_unregister(struct hd_struct *p)
{
	debugfs_remove_recursive(p->debugfs_dir);
	p->debugfs_dir = NULL;
}
