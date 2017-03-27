#ifndef _LINUX_DRIVER_DATA_H
#define _LINUX_DRIVER_DATA_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/gfp.h>
#include <linux/device.h>

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
		void (*found_cb)(const struct firmware *driver_data,
				 void *context);
		void *found_ctx;
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
 * @sync_reqs: synchronization requirements
 *
 * This data structure is intended to carry all requirements and specifications
 * required to complete the task to get the requested driver date file to the
 * caller.
 *
 */
struct driver_data_req_params {
	bool optional;
	struct driver_data_reqs sync_reqs;
	const union driver_data_cbs cbs;
};

#define driver_data_req_param_sync(params)				\
	((params)->sync_reqs.mode == DRIVER_DATA_SYNC)
#define driver_data_req_param_async(params)				\
	((params)->sync_reqs.mode == DRIVER_DATA_ASYNC)

#define driver_data_param_optional(params)	((params)->optional)

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

#endif /* _LINUX_DRIVER_DATA_H */
