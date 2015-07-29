#ifndef _LINUX_DRIVER_DATA_H
#define _LINUX_DRIVER_DATA_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/async.h>

/*
 * Driver Data internals
 *
 * Copyright (C) 2017 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

struct driver_data {
	size_t size;
	const u8 *data;

	/* driver_data loader private fields */
	void *priv;
};

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
				    const struct driver_data *driver_data);
		void *found_ctx;

		int __must_check (*opt_fail_cb)(void *context);
		void *opt_fail_ctx;
	} sync;
	struct {
		void (*found_cb)(const struct driver_data *driver_data,
				 void *context);
		void *found_ctx;

		void (*opt_fail_cb)(void *context);
		void *opt_fail_ctx;
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
 *	found. You must set this to true if you have provided a opt_fail_cb
 *	callback, DRIVER_DATA_SYNC_OPT_CB() and DRIVER_DATA_ASYNC_OPT_CB()
 *	ensures this is done for you. If you set this to true and are using an
 *	asynchronous request but not providing a opt_fail_cb() you should
 *	seriously consider using at the very least using async_cookie provided
 *	to you to driver_data_synchronize_request() to ensure no lingering
 *	requests are kept out of bounds.
 * @keep: if set the caller wants to claim ownership over the driver data
 *	through one of its callbacks, it must later free it with
 *	release_driver_data(). By default this is set to false and the kernel
 *	will release the driver data file for you after callback processing
 *	has completed.
 * @sync_reqs: synchronization requirements, this will be taken care for you
 *	by default if you are usingy driver_data_request(), otherwise you
 *	should provide your own requirements.
 *
 * This structure is set the by the driver and passed to the driver data
 * file helpers driver_data_request() or driver_data_request_async().
 * It is intended to carry all requirements and specifications required
 * to complete the task to get the requested driver date file to the caller.
 * If you wish to extend functionality of driver data file requests you
 * should extend this data structure and make use of the extensions on
 * the callers to avoid unnecessary collateral evolutions.
 *
 * You are allowed to provide a callback to handle if a driver data file was
 * found or not. You do not need to provide a callback. You may also set
 * an optional flag which would enable you to declare that the driver data
 * file is optional and that if it is not found an alternative callback be
 * run for you.
 *
 * Refer to driver_data_request() and driver_data_request_async() for more
 * details.
 */
struct driver_data_req_params {
	bool optional;
	bool keep;
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

#define DRIVER_DATA_KEEP_ASYNC(__found_cb, __ctx)			\
	DRIVER_DATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.keep = true

#define DRIVER_DATA_ASYNC_OPT_CB(__fail_cb, __ctx)			\
	.optional = true,						\
	.cbs.async.opt_fail_cb = __fail_cb,				\
	.cbs.async.opt_fail_ctx = __ctx

#define driver_data_sync_cb(param)   ((params)->cbs.sync.found_cb)
#define driver_data_sync_ctx(params) ((params)->cbs.sync.found_ctx)
static inline
int driver_data_sync_call_cb(const struct driver_data_req_params *params,
			     const struct driver_data *driver_data)
{
	if (params->sync_reqs.mode != DRIVER_DATA_SYNC)
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

#define driver_data_async_cb(params)	((params)->cbs.async.found_cb)
#define driver_data_async_ctx(params)	((params)->cbs.async.found_ctx)
static inline
void driver_data_async_call_cb(const struct driver_data *driver_data,
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

#if defined(CONFIG_FW_LOADER) || \
	(defined(CONFIG_FW_LOADER_MODULE) && defined(MODULE))
int driver_data_request(const char *name,
		    const struct driver_data_req_params *params,
		    struct device *device);
int driver_data_request_async(const char *name,
			  const struct driver_data_req_params *params,
			  struct device *device,
			  async_cookie_t *async_cookie);
void release_driver_data(const struct driver_data *driver_data);
void driver_data_synchronize_request(async_cookie_t async_cookie);
#else
static inline int driver_data_request(const char *name,
				  const struct driver_data_req_params *params,
				  struct device *device)
{
	return -EINVAL;
}

static
inline int driver_data_request_async(const char *name,
				 const struct driver_data_req_params *params,
				 struct device *device,
				 async_cookie_t *async_cookie);
{
	return -EINVAL;
}

static inline void release_driver_data(const struct driver_data *driver_data)
{
}

void driver_data_synchronize_request(async_cookie_t async_cookie)
{
}
#endif

#endif /* _LINUX_DRIVER_DATA_H */
