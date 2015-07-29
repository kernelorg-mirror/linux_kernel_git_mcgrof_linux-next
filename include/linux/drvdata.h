#ifndef _LINUX_DRVDATA_H
#define _LINUX_DRVDATA_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/async.h>

/*
 * Driver Data internals
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

struct drvdata {
	size_t size;
	const u8 *data;

	/* drvdata loader private fields */
	void *priv;
};

/**
 * enum drvdata_mode - driver data mode of operation
 *
 * DRVDATA_SYNC: your call to request driver data is synchronous. We will
 * 	look for the driver data file you have requested immediatley.
 * DRVDATA_ASYNC: your call to request driver data is asynchronous. We will
 * 	schedule the search for your driver data file to be run at a later
 * 	time.
 */
enum drvdata_mode {
	DRVDATA_SYNC,
	DRVDATA_ASYNC,
};

/* one per drvdata_mode */
union drvdata_cbs {
	struct {
		int __must_check (*found_cb)(void *, const struct drvdata *);
		void *found_ctx;

		int __must_check (*opt_fail_cb)(void *);
		void *opt_fail_ctx;
	} sync;
	struct {
		void (*found_cb)(const struct drvdata *, void *);
		void *found_ctx;

		void (*opt_fail_cb)(void *);
		void *opt_fail_ctx;
	} async;
};

struct drvdata_reqs {
	enum drvdata_mode mode;
	struct module *module;
	gfp_t gfp;
};

/**
 * struct drvdata_req_params - driver data request parameters
 * @optional: if true it is not a hard requirement by the caller that this
 *	file be present. An error will not be recorded if the file is not
 *	found. You must set this to true if you have provided a opt_fail_cb
 *	callback, DRVDATA_SYNC_OPT_CB() and DRVDATA_ASYNC_OPT_CB() ensures
 *	this is done for you. If you set this to true and are using an
 *	asynchronous request but not providing a opt_fail_cb() you should
 *	seriously consider using at the very least using async_cookie provided
 *	to you to drvdata_synchronize_request() to ensure no lingering
 *	requests are kept out of bounds.
 * @keep: if set the caller wants to claim ownership over the system data
 *	through one of its callbacks, it must later free it with
 *	release_drvdata(). By default this is set to false and the kernel
 *	will release the system data file for you after callback processing
 *	has completed.
 * @sync_reqs: synchronization requirements, this will be taken care for you
 *	by default if you are usingy drvdata_request(), otherwise you
 *	should provide your own requirements.
 *
 * This structure is set the by the driver and passed to the system data
 * file helpers drvdata_request() or drvdata_request_async().
 * It is intended to carry all requirements and specifications required
 * to complete the task to get the requested system date file to the caller.
 * If you wish to extend functionality of system data file requests you
 * should extend this data structure and make use of the extensions on
 * the callers to avoid unnecessary collateral evolutions.
 *
 * You are allowed to provide a callback to handle if a system data file was
 * found or not. You do not need to provide a callback. You may also set
 * an optional flag which would enable you to declare that the system data
 * file is optional and that if it is not found an alternative callback be
 * run for you.
 *
 * Refer to drvdata_request() and drvdata_request_async() for more
 * details.
 */
struct drvdata_req_params {
	bool optional;
	bool keep;
	struct drvdata_reqs sync_reqs;
	const union drvdata_cbs cbs;
};

/*
 * We keep these template definitions to a minimum for the most
 * popular requests.
 */

/* Typical sync data case */
#define DRVDATA_SYNC_FOUND(__found_cb, __ctx)				\
	.cbs.sync.found_cb = __found_cb,				\
	.cbs.sync.found_ctx = __ctx

#define DRVDATA_DEFAULT_SYNC(__found_cb, __ctx)				\
	DRVDATA_SYNC_FOUND(__found_cb, __ctx)

#define DRVDATA_KEEP_SYNC(__found_cb, __ctx)				\
	DRVDATA_DEFAULT_SYNC(__found_cb, __ctx),			\
	.keep= true

/* If you have one fallback routine */
#define DRVDATA_SYNC_OPT_CB(__fail_cb, __ctx)				\
	.optional = true,						\
	.cbs.sync.opt_fail_cb = __fail_cb,				\
	.cbs.sync.opt_fail_ctx = __ctx

/*
 * Used to define the default asynchronization requirements for
 * drvdata_request_async(). Drivers can override.
 */
#define DRVDATA_DEFAULT_ASYNC(__found_cb, __ctx)			\
	.sync_reqs = {							\
		.mode = DRVDATA_ASYNC,					\
		.module = THIS_MODULE,					\
		.gfp = GFP_KERNEL,					\
	},								\
	.cbs.async = {							\
		.found_cb = __found_cb,					\
		.found_ctx = __ctx,					\
	}

#define DRVDATA_KEEP_ASYNC(__found_cb, __ctx)				\
	DRVDATA_DEFAULT_ASYNC(__found_cb, __ctx),			\
	.keep = true

#define DRVDATA_ASYNC_OPT_CB(__fail_cb, __ctx)				\
	.optional = true,						\
	.cbs.async.opt_fail_cb = __fail_cb,				\
	.cbs.async.opt_fail_ctx = __ctx

#define drvdata_sync_cb(param)		((params)->cbs.sync.found_cb)
#define drvdata_sync_ctx(params)	((params)->cbs.sync.found_ctx)
static inline int drvdata_sync_call_cb(const struct drvdata_req_params *params,
				       const struct drvdata *drvdata)
{
	if (params->sync_reqs.mode != DRVDATA_SYNC)
		return -EINVAL;
	if (!drvdata_sync_cb(params)) {
		if (drvdata)
			return 0;
		return -ENOENT;
	}
	return drvdata_sync_cb(params)(drvdata_sync_ctx(params), drvdata);
}

#define drvdata_sync_opt_cb(params)	((params)->cbs.sync.opt_fail_cb)
#define drvdata_sync_opt_ctx(params)	((params)->cbs.sync.opt_fail_ctx)
static
inline int drvdata_sync_opt_call_cb(const struct drvdata_req_params *params)
{
	if (params->sync_reqs.mode != DRVDATA_SYNC)
		return -EINVAL;
	if (!drvdata_sync_opt_cb(params))
		return 0;
	return drvdata_sync_opt_cb(params)(drvdata_sync_opt_ctx(params));
}

#define drvdata_async_cb(params)	((params)->cbs.async.found_cb)
#define drvdata_async_ctx(params)	((params)->cbs.async.found_ctx)
static
inline void drvdata_async_call_cb(const struct drvdata *drvdata,
				  const struct drvdata_req_params *params)
{
	if (params->sync_reqs.mode != DRVDATA_ASYNC)
		return;
	if (!drvdata_async_cb(params))
		return;
	drvdata_async_cb(params)(drvdata, drvdata_async_ctx(params));
}

#define drvdata_async_opt_cb(params)	((params)->cbs.async.opt_fail_cb)
#define drvdata_async_opt_ctx(params)	((params)->cbs.async.opt_fail_ctx)
static
inline void drvdata_async_opt_call_cb(const struct drvdata_req_params *params)
{
	if (params->sync_reqs.mode != DRVDATA_ASYNC)
		return;
	if (!drvdata_async_opt_cb(params))
		return;
	drvdata_async_opt_cb(params)(drvdata_async_opt_ctx(params));
}

#if defined(CONFIG_FW_LOADER) || (defined(CONFIG_FW_LOADER_MODULE) && defined(MODULE))
int drvdata_request(const char *name,
		    const struct drvdata_req_params *params,
		    struct device *device);
int drvdata_request_async(const char *name,
			  const struct drvdata_req_params *params,
			  struct device *device,
			  async_cookie_t *async_cookie);
void release_drvdata(const struct drvdata *drvdata);
void drvdata_synchronize_request(async_cookie_t async_cookie);
#else
static inline int drvdata_request(const char *name,
				  const struct drvdata_req_params *params,
				  struct device *device)
{
	return -EINVAL;
}

static
inline int drvdata_request_async(const char *name,
				 const struct drvdata_req_params *params,
				 struct device *device,
				 async_cookie_t *async_cookie);
{
	return -EINVAL;
}

static inline void release_drvdata(const struct drvdata *drvdata)
{
}

void drvdata_synchronize_request(async_cookie_t async_cookie)
{
}
#endif

#endif /* _LINUX_DRVDATA_H */
