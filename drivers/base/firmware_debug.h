// SPDX-License-Identifier: GPL-2.0
#ifndef __FIRMWARE_DEBUG_H
#define __FIRMWARE_DEBUG_H

#ifdef CONFIG_FW_LOADER_DEBUG
/**
 * struct firmware_debug - firmware debugging configuration
 *
 * Provided to help debug the firmware API.
 *
 * @force_sysfs_fallback: force the sysfs fallback mechanism to be used
 *	as if one had enabled CONFIG_FW_LOADER_USER_HELPER_FALLBACK=y.
 *	Useful to help debug a CONFIG_FW_LOADER_USER_HELPER_FALLBACK=y
 *	functionality on a kernel where that config entry has been disabled.
 * @ignore_sysfs_fallback: force to disable the sysfs fallback mechanism.
 * 	This emulates the behaviour as if we had set the kernel
 * 	config CONFIG_FW_LOADER_USER_HELPER=n.
 */
struct firmware_debug {
	bool force_sysfs_fallback;
	bool ignore_sysfs_fallback;
};

extern struct firmware_debug fw_debug;

int register_fw_debugfs(void);
void unregister_fw_debugfs(void);

static inline bool fw_debug_force_sysfs_fallback(void)
{
	return fw_debug.force_sysfs_fallback;
}

static inline bool fw_debug_ignore_sysfs_fallback(void)
{
	return fw_debug.ignore_sysfs_fallback;
}
#else
static inline int register_fw_debugfs(void)
{
	return 0;
}
static inline void unregister_fw_debugfs(void)
{
}

static inline bool fw_debug_force_sysfs_fallback(void)
{
	return false;
}

static inline bool fw_debug_ignore_sysfs_fallback(void)
{
	return false;
}
#endif /* CONFIG_FW_LOADER_DEBUG */

#endif /* __FIRMWARE_DEBUG_H */
