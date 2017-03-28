#ifndef _LINUX_DRIVER_DATA_H
#define _LINUX_DRIVER_DATA_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/firmware.h>

/*
 * Driver Data internals
 *
 * Copyright (C) 2017 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

/**
 * struct driver_data_sync_cbs - synchronous driver data callbacks
 * @found_cb: optional callback to be used when the driver data has been found.
 * 	A callback is useful if you wish to take advantage of the feature of
 * 	having your driver_data be released immediately after the callback is
 * 	called, this feature is enabled by default can can be disabled by
 * 	setting the flag %DRIVER_DATA_REQ_KEEP.
 * @found_ctx: preferred context to be used as the second argument to
 * 	@found_cb.
 *
 * Used for specifying callbacks and contexts used for when synchronous driver
 * data requests have completed. If no driver data is found the error will be
 * passed on the respective callback.
 */
struct driver_data_sync_cbs {
	int __must_check
		(*found_cb)(void *context,
			    const struct firmware *driver_data,
			    int error);
	void *found_ctx;

	int __must_check (*opt_fail_cb)(void *context, int error);
	void *opt_fail_ctx;
};

/**
 * struct driver_data_async_cbs - callbacks for handling driver data requests
 * @found_cb: callback to be used when the driver data has been found. A
 *	callback is required. If the requested driver data is found it will
 *	passed on the callback, using the context set on @found_ctx.
 * @found_ctx: preferred context to be used as the second argument to
 * 	@found_cb.
 * @opt_fail_cb: if your driver data is optional and you have a viable approach
 * 	to remedy the lack of finding a driver data with original requirements
 * 	you can implement your solution on this callback.
 * @opt_fail_ctx: context to use for @opt_fail_cb
 * @found_api_cb: callback for the supported version API framework, refer to
 * 	%DRIVER_DATA_REQ_USE_API_VERSIONING for details.
 *
 * Used for specifying callbacks and contexts used for when asynchronous driver
 * data requests have completed. If no driver data is found the error will be
 * passed on the respective callback.
 */
struct driver_data_async_cbs {
	void (*found_cb)(const struct firmware *driver_data,
			 void *context,
			 int error);
	void *found_ctx;

	void (*opt_fail_cb)(void *context, int error);
	void *opt_fail_ctx;

	int __must_check
		(*found_api_cb)(const struct firmware *driver_data,
				void *context, int error);
};

/**
 * union driver_data_cbs - callbacks for driver data request
 * @async: callbacks for handling driver data when asynchronous requests
 * 	are made.
 *
 * Used for placement of callbacks used for handling results from driver
 * data requests.
 */
union driver_data_cbs {
	struct driver_data_async_cbs async;
	struct driver_data_sync_cbs sync;
};

/**
 * enum driver_data_reqs - requirements of the driver data request
 * @DRIVER_DATA_REQ_OPTIONAL: if set it is not a hard requirement by the
 *	caller that the file requested be present. An error will not be recorded
 *	if the file is not found.
 * @DRIVER_DATA_REQ_KEEP: by default the kernel will release the driver data
 *	for you immediately after your respective sync or async callback is
 *	called.  Use this flag to annotate your requirement is for you to keep
 *	and free the driver data on your own. You must free the driver data
 *	using release_driver_data().
 * @DRIVER_DATA_REQ_USE_API_VERSIONING: indicates that the caller has an API
 *	revision system for the the files being requested using a simple
 *	numeric scheme: there is a max API version supported and the lowest API
 *	version supported.  The search starts using the filename requested on
 *	driver_data_request_sync() or driver_data_request_async(), appending
 *	the @api_max to it, and ending with a postfix if api_name_postfix is
 *	specified on &struct driver_data_req_params. If that is not available
 *	it will look for any files with API version lower than this until it
 *	reaches @api_min. This enables chaining driver data requests easily on
 *	behalf of device drivers using a simple one digit versioning scheme.
 *	This feature requires only one file to be present given the API range,
 *	it is only required for one file in the API range to be present.
 *	If the %DRIVER_DATA_REQ_OPTIONAL flag is also enabled then all files
 *	are treated as optional.
 */
enum driver_data_reqs {
	DRIVER_DATA_REQ_OPTIONAL			= 1 << 0,
	DRIVER_DATA_REQ_KEEP				= 1 << 1,
	DRIVER_DATA_REQ_USE_API_VERSIONING		= 1 << 2,
};

/**
 * struct driver_data_req_params - driver data request parameters
 * @hold_module: module to hold during the driver data request operation. By
 * 	default if sync requests set this to NULL the firmware_class module
 * 	will be refcounted during operation.
 * @gfp: flags to use for allocations when constructing the driver data request,
 *	prior to scheduling. Unused on driver_data_request_sync().
 * @reqs: set of &enum driver_data_reqs flags used to configure the driver
 * 	data request. All of the specified requirements must be met.
 * @cbs: set of callbacks to use for the driver data request.
 * @api_min: if uses_api_versioning is set, this represents the lowest
 *	version of API supported by the caller.
 * @api_max: if uses_api_versioning is set, this represents the highest
 *	version of API supported by the caller.
 * @api_name_postfix: use the name as the driver data name prefix, the API
 *	digit will be placed in the middle, followed by the @api_name_postfix.
 * @sync_reqs: synchronization requirements
 *
 * This data structure is intended to carry all requirements and specifications
 * required to complete the task to get the requested driver date file to the
 * caller.
 */
struct driver_data_req_params {
	struct module *hold_module;
	gfp_t gfp;
	u64 reqs;
	u8 api_min;
	u8 api_max;
	const char *api_name_postfix;
	const union driver_data_cbs cbs;
};

/*
 * We keep these template definitions to a minimum for the most
 * popular requests.
 */

/* Typical sync data case */
#define DRIVER_DATA_SYNC_FOUND(__found_cb, __ctx)			\
	.cbs.sync.found_cb = __found_cb,				\
	.cbs.sync.found_ctx = __ctx

#define DRIVER_DATA_DEFAULT_SYNC(__found_cb, __ctx)			\
	DRIVER_DATA_SYNC_FOUND(__found_cb, __ctx)

#define DRIVER_DATA_DEFAULT_SYNC_REQS(__found_cb, __ctx, __reqs)	\
	DRIVER_DATA_SYNC_FOUND(__found_cb, __ctx),			\
	.reqs = (__reqs)

#define DRIVER_DATA_KEEP_SYNC(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_SYNC(__found_cb, __ctx),			\
	.reqs = DRIVER_DATA_REQ_KEEP

/* If you have one fallback routine */
#define DRIVER_DATA_SYNC_OPT_CB(__fail_cb, __ctx)			\
	.reqs = DRIVER_DATA_REQ_OPTIONAL,				\
	.cbs.sync.opt_fail_cb = __fail_cb,				\
	.cbs.sync.opt_fail_ctx = __ctx

#define DRIVER_DATA_SYNC_OPT_CB_REQS(__fail_cb, __ctx, __reqs)		\
	.reqs = DRIVER_DATA_REQ_OPTIONAL | __reqs,			\
	.cbs.sync.opt_fail_cb = __fail_cb,				\
	.cbs.sync.opt_fail_ctx = __ctx

/*
 * Used to define the default asynchronization requirements for
 * driver_data_request_async(). Drivers can override.
 */
#define DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx)			\
	.hold_module = THIS_MODULE,					\
	.gfp = GFP_KERNEL,						\
	.cbs.async = {							\
		.found_cb = __found_cb,					\
		.found_ctx = __ctx,					\
	}

#define DRIVER_DATA_DEFAULT_ASYNC_OPT(__found_cb, __ctx)		\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.reqs = DRIVER_DATA_REQ_OPTIONAL

#define DRIVER_DATA_KEEP_ASYNC(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.reqs = DRIVER_DATA_PRIV_REQ_KEEP

#define DRIVER_DATA_KEEP_ASYNC_OPT(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.reqs = DRIVER_DATA_PRIV_REQ_KEEP |				\
		DRIVER_DATA_REQ_OPTIONAL

#define DRIVER_DATA_ASYNC_OPT_CB(__fail_cb, __ctx)			\
	.reqs = DRIVER_DATA_REQ_OPTIONAL,				\
	.cbs.async.opt_fail_cb = __fail_cb,				\
	.cbs.async.opt_fail_ctx = __ctx

#define DRIVER_DATA_API_CB(__found_api_cb, __ctx)			\
	.hold_module = THIS_MODULE,					\
	.gfp = GFP_KERNEL,						\
	.cbs.async = {							\
		.found_api_cb = __found_api_cb,				\
		.found_ctx = __ctx,					\
	}

#define DRIVER_DATA_API(__min, __max, __postfix)			\
	.reqs = DRIVER_DATA_REQ_USE_API_VERSIONING,			\
	.api_min = __min,						\
	.api_max = __max,						\
	.api_name_postfix = __postfix

#if defined(CONFIG_FW_LOADER) || \
	(defined(CONFIG_FW_LOADER_MODULE) && defined(MODULE))
int driver_data_request_sync(const char *name,
			     const struct driver_data_req_params *params,
			     struct device *device);
int driver_data_request_async(const char *name,
			      const struct driver_data_req_params *params,
			      struct device *device);
#else
static
inline int driver_data_request_sync(const char *name,
				    const struct driver_data_req_params *params,
				    struct device *device)
{
	return -EINVAL;
}

static
inline int driver_data_request_async(const char *name,
				 const struct driver_data_req_params *params,
				 struct device *device)
{
	return -EINVAL;
}
#endif

#endif /* _LINUX_DRIVER_DATA_H */
