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
 * enum driver_data_mode - driver data mode of operation
 *
 * DRIVER_DATA_SYNC: your call to request driver data is synchronous. We will
 *	look for the driver data file you have requested immediatley.
 * DRIVER_DATA_ASYNC: your call to request driver data is asynchronous. We will
 *	schedule the search for your driver data file to be run at a later
 *	time.
 */
enum driver_data_mode {
	DRIVER_DATA_SYNC,
	DRIVER_DATA_ASYNC,
};

/* one per driver_data_mode */
union driver_data_cbs {
	struct {
		int __must_check
			(*found_cb)(void *context,
				    const struct firmware *driver_data);
		void *found_ctx;

		int __must_check (*opt_fail_cb)(void *context);
		void *opt_fail_ctx;
	} sync;
	struct {
		void (*found_cb)(const struct firmware *driver_data,
				 void *context);
		void *found_ctx;

		void (*opt_fail_cb)(void *context);
		void *opt_fail_ctx;

		int __must_check
			(*found_api_cb)(const struct firmware *driver_data,
					void *context);
	} async;
};

struct driver_data_reqs {
	enum driver_data_mode mode;
	struct module *module;
	gfp_t gfp;
};

/**
 * struct driver_data_req_params - driver data request parameters
 * @optional: if true it is not a hard requirement by the caller that this
 *	file be present. An error will not be recorded if the file is not
 *	found.
 * @keep: if set the caller wants to claim ownership over the driver data
 *	through one of its callbacks, it must later free it with
 *	release_driver_data(). By default this is set to false and the kernel
 *	will release the driver data file for you after callback processing
 *	has completed.
 * @uses_api_versioning: if set the caller is indicating that the caller has
 * 	an API revision system for the the files being requested using a
 * 	simple numeric scheme: there is a max API version supported and the
 * 	lowest API version supported. When used the driver data request request
 * 	will always look for the filename requested but by first appeneding the
 * 	@api_max to it, and looking for that. If that is not available it will
 * 	look for any files with API version lower than this until it reaches
 * 	@api_min. This enables chaining driver data requests easily on behalf
 * 	of device drivers using a simple one digit versioning scheme. When
 * 	this is true it will treat all files not found as non-fatal, as
 * 	optional, but it requires at least one file to be found. If the
 * 	@optional attribute is also true then if no files are found we won't
 * 	complain at all.
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
 *
 */
struct driver_data_req_params {
	bool optional;
	bool keep;
	bool uses_api_versioning;
	u8 api_min;
	u8 api_max;
	const char *api_name_postfix;
	struct driver_data_reqs sync_reqs;
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

#define DRIVER_DATA_KEEP_SYNC(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_SYNC(__found_cb, __ctx),			\
	.keep = true

/* If you have one fallback routine */
#define DRIVER_DATA_SYNC_OPT_CB(__fail_cb, __ctx)			\
	.optional = true,						\
	.cbs.sync.opt_fail_cb = __fail_cb,				\
	.cbs.sync.opt_fail_ctx = __ctx

/*
 * Used to define the default asynchronization requirements for
 * driver_data_request_async(). Drivers can override.
 */
#define DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx)			\
	.sync_reqs = {							\
		.mode = DRIVER_DATA_ASYNC,				\
		.module = THIS_MODULE,					\
		.gfp = GFP_KERNEL,					\
	},								\
	.cbs.async = {							\
		.found_cb = __found_cb,					\
		.found_ctx = __ctx,					\
	}

#define DRIVER_DATA_DEFAULT_ASYNC_OPT(__found_cb, __ctx)		\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.optional = true

#define DRIVER_DATA_KEEP_ASYNC(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.keep = true

#define DRIVER_DATA_KEEP_ASYNC_OPT(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.optional = true

#define DRIVER_DATA_ASYNC_OPT_CB(__fail_cb, __ctx)			\
	.optional = true,						\
	.cbs.async.opt_fail_cb = __fail_cb,				\
	.cbs.async.opt_fail_ctx = __ctx

#define DRIVER_DATA_API_CB(__found_api_cb, __ctx)			\
	.sync_reqs = {							\
		.mode = DRIVER_DATA_ASYNC,				\
		.module = THIS_MODULE,					\
		.gfp = GFP_KERNEL,					\
	},								\
	.cbs.async = {							\
		.found_api_cb = __found_api_cb,				\
		.found_ctx = __ctx,					\
	}

#define DRIVER_DATA_API(__min, __max, __postfix)			\
	.uses_api_versioning = true,					\
	.api_min = __min,						\
	.api_max = __max,						\
	.api_name_postfix = __postfix

#define driver_data_req_param_sync(params)				\
	((params)->sync_reqs.mode == DRIVER_DATA_SYNC)
#define driver_data_req_param_async(params)				\
	((params)->sync_reqs.mode == DRIVER_DATA_ASYNC)

#define driver_data_param_optional(params)	((params)->optional)
#define driver_data_param_keep(params)		((params)->keep)
#define driver_data_param_uses_api(params)	((params)->uses_api_versioning)

#define driver_data_sync_cb(param)   ((params)->cbs.sync.found_cb)
#define driver_data_sync_ctx(params) ((params)->cbs.sync.found_ctx)
static inline
int driver_data_sync_call_cb(const struct driver_data_req_params *params,
			     const struct firmware *driver_data)
{
	if (!driver_data_req_param_sync(params))
		return -EINVAL;
	if (!driver_data_sync_cb(params)) {
		if (driver_data)
			return 0;
		return -ENOENT;
	}
	return driver_data_sync_cb(params)(driver_data_sync_ctx(params),
					   driver_data);
}

#define driver_data_sync_opt_cb(params)  ((params)->cbs.sync.opt_fail_cb)
#define driver_data_sync_opt_ctx(params) ((params)->cbs.sync.opt_fail_ctx)
static inline
int driver_data_sync_opt_call_cb(const struct driver_data_req_params *params)
{
	if (params->sync_reqs.mode != DRIVER_DATA_SYNC)
		return -EINVAL;
	if (!driver_data_sync_opt_cb(params))
		return 0;
	return driver_data_sync_opt_cb(params)
		(driver_data_sync_opt_ctx(params));
}

#define driver_data_async_cb(params)		((params)->cbs.async.found_cb)
#define driver_data_async_ctx(params)		((params)->cbs.async.found_ctx)
static inline
void driver_data_async_call_cb(const struct firmware *driver_data,
			       const struct driver_data_req_params *params)
{
	if (params->sync_reqs.mode != DRIVER_DATA_ASYNC)
		return;
	if (!driver_data_async_cb(params))
		return;
	driver_data_async_cb(params)(driver_data,
				     driver_data_async_ctx(params));
}

#define driver_data_async_opt_cb(params)  ((params)->cbs.async.opt_fail_cb)
#define driver_data_async_opt_ctx(params) ((params)->cbs.async.opt_fail_ctx)
static inline
void driver_data_async_opt_call_cb(const struct driver_data_req_params *params)
{
	if (params->sync_reqs.mode != DRIVER_DATA_ASYNC)
		return;
	if (!driver_data_async_opt_cb(params))
		return;
	driver_data_async_opt_cb(params)(driver_data_async_opt_ctx(params));
}

#define driver_data_async_api_cb(params)	((params)->cbs.async.found_api_cb)
static inline
int driver_data_async_call_api_cb(const struct firmware *driver_data,
				  const struct driver_data_req_params *params)
{
	if (!params->uses_api_versioning)
		return -EINVAL;
	if (!driver_data_async_api_cb(params))
		return -EINVAL;
	return driver_data_async_api_cb(params)(driver_data,
						driver_data_async_ctx(params));
}

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
