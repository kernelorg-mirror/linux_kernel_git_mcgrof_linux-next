// SPDX-License-Identifier: GPL-2.0

/*
 * Shared request-based / make_request-based functionality
 */
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>

struct dentry *blk_debugfs_root;
struct dentry *blk_debugfs_bsg = NULL;

/**
 * enum blk_debugfs_dir_type - block device debugfs directory type
 * @BLK_DBG_DIR_BASE: the block device debugfs_dir exists on the base
 * 	system <system-debugfs-dir>/block/ debugfs directory.
 * @BLK_DBG_DIR_BSG: the block device debugfs_dir is under the directory
 * 	<system-debugfs-dir>/block/bsg/
 */
enum blk_debugfs_dir_type {
	BLK_DBG_DIR_BASE = 1,
	BLK_DBG_DIR_BSG,
};

void blk_debugfs_register(void)
{
	blk_debugfs_root = debugfs_create_dir("block", NULL);
}

static struct dentry *queue_get_base_dir(enum blk_debugfs_dir_type type)
{
	switch (type) {
	case BLK_DBG_DIR_BASE:
		return blk_debugfs_root;
	case BLK_DBG_DIR_BSG:
		return blk_debugfs_bsg;
	}
	return NULL;
}

static void queue_debugfs_register_type(struct request_queue *q,
					const char *name,
					enum blk_debugfs_dir_type type)
{
	struct dentry *base_dir = queue_get_base_dir(type);

	q->debugfs_dir = debugfs_create_dir(name, base_dir);
}

/**
 * blk_queue_debugfs_register - register the debugfs_dir for the block device
 * @q: the associated request_queue of the block device
 * @name: the name of the block device exposed
 *
 * This is used to create the debugfs_dir used by the block layer and blktrace.
 * Drivers which use any of the *add_disk*() calls or variants have this called
 * automatically for them. This directory is removed automatically on
 * blk_release_queue() once the request_queue reference count reaches 0.
 */
void blk_queue_debugfs_register(struct request_queue *q, const char *name)
{
	queue_debugfs_register_type(q, name, BLK_DBG_DIR_BASE);
}
EXPORT_SYMBOL_GPL(blk_queue_debugfs_register);

/**
 * blk_queue_debugfs_unregister - remove the debugfs_dir for the block device
 * @q: the associated request_queue of the block device
 *
 * Removes the debugfs_dir for the request_queue on the associated block device.
 * This is handled for you on blk_release_queue(), and that should only be
 * called once.
 *
 * Since we don't care where the debugfs_dir was created this is used for all
 * types of of enum blk_debugfs_dir_type.
 */
void blk_queue_debugfs_unregister(struct request_queue *q)
{
	debugfs_remove_recursive(q->debugfs_dir);
}

static struct dentry *queue_debugfs_symlink_type(struct request_queue *q,
						 const char *src,
						 const char *dst,
						 enum blk_debugfs_dir_type type)
{
	struct dentry *dentry = ERR_PTR(-EINVAL);
	char *dir_dst;

	dir_dst = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!dir_dst)
		return dentry;

	switch (type) {
	case BLK_DBG_DIR_BASE:
		if (dst)
			snprintf(dir_dst, PATH_MAX, "%s", dst);
		else if (!IS_ERR_OR_NULL(q->debugfs_dir))
			snprintf(dir_dst, PATH_MAX, "%s",
				 q->debugfs_dir->d_name.name);
		else
			goto out;
		break;
	case BLK_DBG_DIR_BSG:
		if (dst)
			snprintf(dir_dst, PATH_MAX, "bsg/%s", dst);
		else
			goto out;
		break;
	}

	/*
	 * The base block debugfs directory is always used for the symlinks,
	 * their target is what changes.
	 */
	dentry = debugfs_create_symlink(src, blk_debugfs_root, dir_dst);
out:
	kfree(dir_dst);

	return dentry;
}

/**
 * blk_queue_debugfs_symlink - symlink to the real block device debugfs_dir
 * @q: the request queue where we know the debugfs_dir exists or will exist
 *     eventually. Cannot be NULL.
 * @src: name of the exposed device we wish to associate to the block device
 * @dst: the name of the directory to which we want to symlink to, may be NULL
 *	 if you do not know what this may be, but only if your base block device
 *	 is not bsg. If you set this to NULL, we will have no other option but
 *	 to look at the request_queue to infer the name, but you must ensure
 *	 it is already be set, be mindful of asynchronous probes.
 *
 * Some devices don't have a request_queue of their own, however, they have an
 * association to one and have historically supported using the same
 * debugfs_dir which has been used to represent the whole disk for blktrace
 * functionality. Such is the case for partitions and for scsi-generic devices.
 * They share the same request_queue and debugfs_dir as with the whole disk for
 * blktrace purposes.  This helper allows such association to be made explicit
 * and enable blktrace functionality for them. scsi-generic devices representing
 * scsi device such as block, cdrom, tape, media changer register their own
 * debug_dir already and share the same request_queue as with scsi-generic, as
 * such the respective scsi-generic debugfs_dir is just a symlink to these
 * driver's debugfs_dir.
 *
 * To remove use debugfs_remove() on the symlink dentry returned by this
 * function. The block layer will not clean this up for you, you must remove
 * it yourself in case of device removal.
 */
struct dentry *blk_queue_debugfs_symlink(struct request_queue *q,
					 const char *src,
					 const char *dst)
{
	return queue_debugfs_symlink_type(q, src, dst, BLK_DBG_DIR_BASE);
}
EXPORT_SYMBOL_GPL(blk_queue_debugfs_symlink);

#ifdef CONFIG_BLK_DEV_BSG

void blk_debugfs_register_bsg(void)
{
	blk_debugfs_bsg = debugfs_create_dir("bsg", blk_debugfs_root);
}

/**
 * blk_queue_debugfs_register_bsg - create the debugfs_dir for bsg block devices
 * @q: the associated request_queue of the block device
 * @name: the name of the block device exposed
 *
 * This is used to create the debugfs_dir used by the Block layer SCSI generic
 * (bsg) driver. This is to be used only by the scsi-generic driver on behalf
 * of scsi devices which work as scsi controllers or transports.
 *
 * This directory is cleaned up for all drivers automatically on
 * blk_release_queue() once the request_queue reference count reaches 0.
 */
void blk_queue_debugfs_register_bsg(struct request_queue *q, const char *name)
{
	queue_debugfs_register_type(q, name, BLK_DBG_DIR_BSG);
}
EXPORT_SYMBOL_GPL(blk_queue_debugfs_register_bsg);

/**
 * blk_queue_debugfs_symlink_bsg - symlink to the bsg debugfs_dir
 * @q: the request queue where we know the debugfs_dir exists or will exist
 *     eventually. Cannot be NULL.
 * @src: name of the scsi-generic device we wish to associate to the bsg
 * 	request_queue.
 * @dst: the name of the bsg request_queue debugfs_dir to which we want to
 *	 symlink to. This cannot be NULL.
 *
 * This is used by scsi-generic devices representing raid controllers /
 * transport drivers.
 */
struct dentry *blk_queue_debugfs_bsg_symlink(struct request_queue *q,
					     const char *src,
					     const char *dst)
{
	return queue_debugfs_symlink_type(q, src, dst, BLK_DBG_DIR_BSG);
}
EXPORT_SYMBOL_GPL(blk_queue_debugfs_bsg_symlink);
#endif /* CONFIG_BLK_DEV_BSG */
