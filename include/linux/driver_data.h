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
 * struct driver_data_async_cbs - callbacks for handling driver data requests
 * @found_cb: callback to be used when the driver data has been found. A
 *	callback is required. If the requested driver data is found it will
 *	passed on the callback, using the context set on @found_ctx.
 * @found_ctx: preferred context to be used as the second argument to
 * 	@found_cb.
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
};

/**
 * enum driver_data_reqs - requirements of the driver data request
 * @DRIVER_DATA_REQ_OPTIONAL: if set it is not a hard requirement by the
 *	caller that the file requested be present. An error will not be recorded
 *	if the file is not found.
 */
enum driver_data_reqs {
	DRIVER_DATA_REQ_OPTIONAL			= 1 << 0,
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
 *
 * This data structure is intended to carry all requirements and specifications
 * required to complete the task to get the requested driver date file to the
 * caller.
 */
struct driver_data_req_params {
	struct module *hold_module;
	gfp_t gfp;
	u64 reqs;
	const union driver_data_cbs cbs;
};

#endif /* _LINUX_DRIVER_DATA_H */
