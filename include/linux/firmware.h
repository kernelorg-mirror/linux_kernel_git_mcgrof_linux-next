/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FIRMWARE_H
#define _LINUX_FIRMWARE_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/gfp.h>

#define FW_ACTION_NOHOTPLUG 0
#define FW_ACTION_HOTPLUG 1

struct firmware {
	size_t size;
	const u8 *data;
	struct page **pages;

	/* firmware loader private fields */
	void *priv;
};

struct module;
struct device;

struct builtin_fw {
	char *name;
	void *data;
	unsigned long size;
};

/**
 * struct fw_async_cbs - callbacks for handling firmware requests
 * @found_cb: callback to be used when the firmware has been found. A
 *	callback is required. If the requested firmware is found it will
 *	passed on the callback, using the context set on @found_ctx.
 * @found_ctx: preferred context to be used as the second argument to
 * 	@found_cb.
 *
 * Used for specifying callbacks and contexts used for when asynchronous
 * firmware requests have completed. If no firmware is found the error will be
 * passed on the respective callback.
 */
struct fw_async_cbs {
	void (*found_cb)(const struct firmware *fw,
			 void *context,
			 int error);
	void *found_ctx;
};

/**
 * union fw_cbs - callbacks for firmware request
 * @async: callbacks for handling firmware when asynchronous requests
 * 	are made.
 *
 * Used for placement of callbacks used for handling results from firmware
 * requests.
 */
union fw_cbs {
	struct fw_async_cbs async;
};

/**
 * enum fw_reqs - requirements of the firmware request
 * @FW_REQ_OPTIONAL: if set it is not a hard requirement by the
 *	caller that the file requested be present. An error will not be recorded
 *	if the file is not found.
 */
enum fw_reqs {
	FW_REQ_OPTIONAL			= 1 << 0,
};

/**
 * struct fw_req_params - firmware request parameters
 * @hold_module: module to hold during the firmware request operation. By
 * 	default if sync requests set this to NULL the firmware_class module
 * 	will be refcounted during operation.
 * @gfp: flags to use for allocations when constructing the firmware request,
 *	prior to scheduling. Unused on fw_request_sync().
 * @reqs: set of &enum fw_reqs flags used to configure the firmware
 * 	request. All of the specified requirements must be met.
 * @cbs: set of callbacks to use for the firmware request.
 *
 * This data structure is intended to carry all requirements and specifications
 * required to complete the task to get the requested driver date file to the
 * caller.
 */
struct fw_req_params {
	struct module *hold_module;
	gfp_t gfp;
	u64 reqs;
	const union fw_cbs cbs;
};

/* We have to play tricks here much like stringify() to get the
   __COUNTER__ macro to be expanded as we want it */
#define __fw_concat1(x, y) x##y
#define __fw_concat(x, y) __fw_concat1(x, y)

#define DECLARE_BUILTIN_FIRMWARE(name, blob)				     \
	DECLARE_BUILTIN_FIRMWARE_SIZE(name, &(blob), sizeof(blob))

#define DECLARE_BUILTIN_FIRMWARE_SIZE(name, blob, size)			     \
	static const struct builtin_fw __fw_concat(__builtin_fw,__COUNTER__) \
	__used __section(.builtin_fw) = { name, blob, size }

#if defined(CONFIG_FW_LOADER) || (defined(CONFIG_FW_LOADER_MODULE) && defined(MODULE))
int request_firmware(const struct firmware **fw, const char *name,
		     struct device *device);
int request_firmware_nowait(
	struct module *module, bool uevent,
	const char *name, struct device *device, gfp_t gfp, void *context,
	void (*cont)(const struct firmware *fw, void *context));
int request_firmware_direct(const struct firmware **fw, const char *name,
			    struct device *device);
int request_firmware_into_buf(const struct firmware **firmware_p,
	const char *name, struct device *device, void *buf, size_t size);

void release_firmware(const struct firmware *fw);
#else
static inline int request_firmware(const struct firmware **fw,
				   const char *name,
				   struct device *device)
{
	return -EINVAL;
}
static inline int request_firmware_nowait(
	struct module *module, bool uevent,
	const char *name, struct device *device, gfp_t gfp, void *context,
	void (*cont)(const struct firmware *fw, void *context))
{
	return -EINVAL;
}

static inline void release_firmware(const struct firmware *fw)
{
}

static inline int request_firmware_direct(const struct firmware **fw,
					  const char *name,
					  struct device *device)
{
	return -EINVAL;
}

static inline int request_firmware_into_buf(const struct firmware **firmware_p,
	const char *name, struct device *device, void *buf, size_t size)
{
	return -EINVAL;
}

#endif
#endif
