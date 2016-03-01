/*
 * Driver data test interface
 *
 * Copyright (C) 2017 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 *
 * This module provides an interface to trigger and test the driver data API
 * through a series of configurations and a few triggers. This driver
 * lacks any extra dependencies, and will not normally be loaded by the
 * system unless explicitly requested by name. You can also build this
 * driver into your kernel.
 *
 * Although all configurations are already written for and will be supported
 * for this test driver, ideally we should strive to see what mechanisms we
 * can put in place to instead automatically generate this sort of test
 * interface, test cases, and infer results. Its a simple enough interface that
 * should hopefully enable more exploring in this area.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/completion.h>
#include <linux/driver_data.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/async.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>

/* Used for the fallback default to test against */
#define TEST_DRIVER_DATA "test-driver_data.bin"

/*
 * For device allocation / registration
 */
static DEFINE_MUTEX(reg_dev_mutex);
static LIST_HEAD(reg_test_devs);

/*
 * num_test_devs actually represents the *next* ID of the next
 * device we will allow to create.
 */
int num_test_devs;

/**
 * test_config - represents configuration for the driver_data API
 *
 * @name: the name of the primary driver_data file to look for
 * @default_name: a fallback example, used to test the optional callback
 *	mechanism.
 * @async: true if you want to trigger an async request. This will use
 *	driver_data_request_async(). If false the synchronous call will
 *	be used, driver_data_request_sync().
 * @optional: whether or not the driver_data is optional refer to the
 *	struct driver_data_reg_params @optional field for more information.
 * @keep: whether or not we wish to free the driver_data on our own, refer to
 *	the struct driver_data_req_params @keep field for more information.
 * @enable_opt_cb: whether or not the optional callback should be set
 *	on a trigger. There is no equivalent setting on the struct
 *	driver_data_req_params as this is implementation specific, and in
 *	in driver_data API its explicit if you had defined an optional call
 *	back for your descriptor with either DRIVER_DATA_SYNC_OPT_CB() or
 *	DRIVER_DATA_ASYNC_OPT_CB(). Since the params are in a const we have
 *	no option but to use a flag and two const structs to decide which
 *	one we should use.
 * @use_api_versioning: use the driver data API versioning support. This
 *	currenlty implies you are using an async test.
 * @api_min: API min version to use for the test.
 * @api_max: API max version to use for the test.
 * @api_name_postfix: API name postfix
 * @test_result: a test may use this to collect the result from the call
 *	of the driver_data_request_async() or driver_data_request_sync() calls
 *	used in their tests. Note that for async calls this typically will be a
 *	successful result (0) unless of course you've used bogus parameters, or
 *	the system is out of memory. Tests against the callbacks can only be
 *	implementation specific, so we don't test for that for now but it may
 *	make sense to build tests cases against a series of semantically
 *	similar family of callbacks that generally represents usage in the
 *	kernel. Synchronous calls return bogus error checks against the
 *	parameters as well, but also return the result of the work from the
 *	callbacks. You can therefore rely on sync calls if you really want to
 *	test for the callback results as well. Errors you can expect:
 *
 *	API specific:
 *
 *	0:		success for sync, for async it means request was sent
 *	-EINVAL:	invalid parameters or request
 *	-ENOENT:	files not found
 *
 *	System environment:
 *
 *	-ENOMEM:	memory pressure on system
 *	-ENODEV:	out of number of devices to test
 *
 * The ordering of elements in this struct must match the exact order of the
 * elements in the ATTRIBUTE_GROUPS(test_dev_config), this is done to know
 * what corresponding field each device attribute configuration entry maps
 * to what struct member on test_alloc_dev_attrs().
 */
struct test_config {
	char *name;
	char *default_name;
	bool async;
	bool optional;
	bool keep;
	bool enable_opt_cb;
	bool use_api_versioning;
	u8 api_min;
	u8 api_max;
	char *api_name_postfix;

	int test_result;
};

/**
 * test_driver_data_private - private device driver driver_data representation
 *
 * @size: size of the data copied, in bytes
 * @data: the actual data we copied over from driver_data
 * @written: true if a callback managed to copy data over to the device
 *	successfully. Since different callbacks are used for this purpose
 *	having the data written does not necessarily mean a test case
 *	completed successfully. Each tests case has its own specific
 *	goals.
 *
 * Private representation of buffer where we put the device system data.
 */
struct test_driver_data_private {
	size_t size;
	u8 *data;
	u8 api;
	bool written;
};

/**
 * driver_data_test_device - test device to help test driver_data
 *
 * @dev_idx: unique ID for test device
 * @config: this keeps the device's own configuration. Instead of creating
 *	different triggers for all possible test cases we can think of in
 *	kernel, we expose a set possible device attributes for tuning the
 *	driver_data API and we to let you tune them in userspace. We then just
 *	provide one trigger.
 * @test_driver_data: internal private representation of a storage area
 *	a driver might typically use to stuff firmware / driver_data.
 * @misc_dev: we use a misc device under the hood
 * @dev: pointer to misc_dev's own struct device
 * @api_found_calls: number of calls a fetch for a driver was found. We use
 *	for internal use on the api callback.
 * @driver_data_mutex: for access into the @driver_data, the fake storage
 *	location for the system data we copy.
 * @config_mutex: used to protect configuration changes
 * @trigger_mutex: all triggers are mutually exclusive when testing. To help
 *	enable testing you can create a different device, each device has its
 *	own set of protections, mimicking real devices.
 * @request_complete: used to help the driver inform itself when async
 * 	callbacks complete.
 * list: needed to be part of the reg_test_devs
 */
struct driver_data_test_device {
	int dev_idx;
	struct test_config config;
	struct test_driver_data_private test_driver_data;
	struct miscdevice misc_dev;
	struct device *dev;

	u8 api_found_calls;

	struct mutex driver_data_mutex;
	struct mutex config_mutex;
	struct mutex trigger_mutex;
	struct completion request_complete;
	struct list_head list;
};

static struct miscdevice *dev_to_misc_dev(struct device *dev)
{
	return dev_get_drvdata(dev);
}

static struct driver_data_test_device *
misc_dev_to_test_dev(struct miscdevice *misc_dev)
{
	return container_of(misc_dev, struct driver_data_test_device, misc_dev);
}

static struct driver_data_test_device *dev_to_test_dev(struct device *dev)
{
	struct miscdevice *misc_dev;

	misc_dev = dev_to_misc_dev(dev);

	return misc_dev_to_test_dev(misc_dev);
}

static ssize_t test_fw_misc_read(struct file *f, char __user *buf,
				 size_t size, loff_t *offset)
{
	struct miscdevice *misc_dev = f->private_data;
	struct driver_data_test_device *test_dev =
		misc_dev_to_test_dev(misc_dev);
	struct test_driver_data_private *test_driver_data =
		&test_dev->test_driver_data;
	ssize_t ret = 0;

	mutex_lock(&test_dev->driver_data_mutex);
	if (test_driver_data->written)
		ret = simple_read_from_buffer(buf, size, offset,
					      test_driver_data->data,
					      test_driver_data->size);
	mutex_unlock(&test_dev->driver_data_mutex);

	return ret;
}

static const struct file_operations test_fw_fops = {
	.owner          = THIS_MODULE,
	.read           = test_fw_misc_read,
};

static
void free_test_driver_data(struct test_driver_data_private *test_driver_data)
{
	kfree(test_driver_data->data);
	test_driver_data->data = NULL;
	test_driver_data->size = 0;
	test_driver_data->api = 0;
	test_driver_data->written = false;
}

static int test_load_driver_data(struct driver_data_test_device *test_dev,
				 const struct firmware *driver_data)
{
	struct test_driver_data_private *test_driver_data =
		&test_dev->test_driver_data;
	int ret = 0;

	if (!driver_data)
		return -ENOENT;

	mutex_lock(&test_dev->driver_data_mutex);

	free_test_driver_data(test_driver_data);

	test_driver_data->data = kzalloc(driver_data->size, GFP_KERNEL);
	if (!test_driver_data->data) {
		ret = -ENOMEM;
		goto out;
	}

	memcpy(test_driver_data->data, driver_data->data, driver_data->size);
	test_driver_data->size = driver_data->size;
	test_driver_data->written = true;
	test_driver_data->api = driver_data->api;

	dev_info(test_dev->dev, "loaded: %zu\n", test_driver_data->size);

out:
	mutex_unlock(&test_dev->driver_data_mutex);

	return ret;
}

static int sync_found_cb(void *context, const struct firmware *driver_data,
			 int unused_error)
{
	struct driver_data_test_device *test_dev = context;
	int ret;

	ret = test_load_driver_data(test_dev, driver_data);
	if (ret)
		dev_info(test_dev->dev,
			 "unable to write driver_data: %d\n", ret);
	return ret;
}

static ssize_t config_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int len = 0;

	mutex_lock(&test_dev->config_mutex);

	len += snprintf(buf, PAGE_SIZE,
			"Custom trigger configuration for: %s\n",
			dev_name(dev));

	if (config->default_name)
		len += snprintf(buf+len, PAGE_SIZE,
				"default name:\t%s\n",
				config->default_name);
	else
		len += snprintf(buf+len, PAGE_SIZE,
				"default name:\tEMTPY\n");

	if (config->name)
		len += snprintf(buf+len, PAGE_SIZE,
				"name:\t\t%s\n", config->name);
	else
		len += snprintf(buf+len, PAGE_SIZE,
				"name:\t\tEMPTY\n");

	len += snprintf(buf+len, PAGE_SIZE,
			"type:\t\t%s\n",
			config->async ? "async" : "sync");
	len += snprintf(buf+len, PAGE_SIZE,
			"optional:\t%s\n",
			config->optional ? "true" : "false");
	len += snprintf(buf+len, PAGE_SIZE,
			"enable_opt_cb:\t%s\n",
			config->enable_opt_cb ? "true" : "false");
	len += snprintf(buf+len, PAGE_SIZE,
			"use_api_versioning:\t%s\n",
			config->use_api_versioning ? "true" : "false");
	len += snprintf(buf+len, PAGE_SIZE,
			"api_min:\t%u\n", config->api_min);
	len += snprintf(buf+len, PAGE_SIZE,
			"api_max:\t%u\n", config->api_max);
	if (config->api_name_postfix)
		len += snprintf(buf+len, PAGE_SIZE,
				"api_name_postfix:\t\t%s\n", config->api_name_postfix);
	else
		len += snprintf(buf+len, PAGE_SIZE,
				"api_name_postfix:\t\tEMPTY\n");
	len += snprintf(buf+len, PAGE_SIZE,
			"keep:\t\t%s\n",
			config->keep ? "true" : "false");

	mutex_unlock(&test_dev->config_mutex);

	return len;
}
static DEVICE_ATTR_RO(config);

static int config_load_data(struct driver_data_test_device *test_dev,
			    const struct firmware *driver_data)
{
	struct test_config *config = &test_dev->config;
	int ret;

	ret = test_load_driver_data(test_dev, driver_data);
	if (ret) {
		if (!config->optional)
			dev_info(test_dev->dev,
				 "unable to write driver_data\n");
	}
	if (config->keep) {
		release_firmware(driver_data);
		driver_data = NULL;
	}

	return ret;
}

static int config_req_default(struct driver_data_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;
	int ret;
	/*
	 * Note: we don't chain config->optional here, we make this
	 * fallback file a requirement. It doesn't make much sense to test
	 * chaining further as the optional callback is implementation
	 * specific, by testing it once we test it for any possible
	 * chains. We provide this as an example of what people can do
	 * and use a default non-optional fallback.
	 */
	const struct driver_data_req_params req_params = {
		DRIVER_DATA_DEFAULT_SYNC(sync_found_cb, test_dev),
	};

	if (config->async)
		dev_info(test_dev->dev,
			 "loading default fallback '%s' using sync request now\n",
			 config->default_name);
	else
		dev_info(test_dev->dev,
			 "loading default fallback '%s'\n",
			 config->default_name);

	ret = driver_data_request_sync(config->default_name,
				       &req_params, test_dev->dev);
	if (ret)
		dev_info(test_dev->dev,
			 "load of default '%s' failed: %d\n",
			 config->default_name, ret);

	return ret;
}

/*
 * This is the default sync fallback callback, as a fallback this
 * then uses a sync request.
 */
static int config_sync_req_default_cb(void *context, int unused_error)
{
	struct driver_data_test_device *test_dev = context;
	int ret;

	ret = config_req_default(test_dev);

	return ret;

	/* Leave all the error checking for the main caller */
}

/*
 * This is the default config->async fallback callback, as a fallback this
 * then uses a sync request.
 */
static void config_async_req_default_cb(void *context, int unused_error)
{
	struct driver_data_test_device *test_dev = context;

	config_req_default(test_dev);

	complete(&test_dev->request_complete);
	/* Leave all the error checking for the main caller */
}

static int config_sync_req_cb(void *context,
			      const struct firmware *driver_data,
			      int unused_error)
{
	struct driver_data_test_device *test_dev = context;

	return config_load_data(test_dev, driver_data);
}

static int trigger_config_sync(struct driver_data_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;
	int ret;
	const struct driver_data_req_params req_params_default = {
		DRIVER_DATA_DEFAULT_SYNC_REQS(config_sync_req_cb, test_dev,
					      DRIVER_DATA_REQ_OPTIONAL |
					      DRIVER_DATA_REQ_KEEP)
	};
	const struct driver_data_req_params req_params_opt_cb = {
		DRIVER_DATA_DEFAULT_SYNC(config_sync_req_cb, test_dev),
		DRIVER_DATA_SYNC_OPT_CB(config_sync_req_default_cb,
					     test_dev),
		.reqs = (config->optional ? DRIVER_DATA_REQ_OPTIONAL : 0) |
			(config->keep ? DRIVER_DATA_REQ_KEEP : 0),
	};
	const struct driver_data_req_params *req_params;

	if (config->enable_opt_cb)
		req_params = &req_params_opt_cb;
	else
		req_params = &req_params_default;

	ret = driver_data_request_sync(config->name, req_params, test_dev->dev);
	if (ret)
		dev_err(test_dev->dev, "sync load of '%s' failed: %d\n",
			config->name, ret);

	return ret;
}

static void config_async_req_cb(const struct firmware *driver_data,
				void *context, int unused_error)
{
	struct driver_data_test_device *test_dev = context;

	config_load_data(test_dev, driver_data);
	complete(&test_dev->request_complete);
}

static int config_async_req_api_cb(const struct firmware *driver_data,
				   void *context,
				   int unused_error)
{
	struct driver_data_test_device *test_dev = context;
	/*
	 * This drivers may process a file and determine it does not
	 * like it, so it wants us to try again, to do this it returns
	 * -EAGAIN. We mimick this behaviour by not liking odd numbered
	 * api files, so we know to expect only success on even numbered
	 * apis.
	 */
	if (driver_data && (driver_data->api % 2 == 1)) {
		pr_info("File api %u found but we purposely ignore it\n",
			driver_data->api);
		return -EAGAIN;
	}

	config_load_data(test_dev, driver_data);

	/*
	 * If the file was found we let our stupid driver emulator thing
	 * fake holding the driver data. If the file was not found just
	 * bail immediately.
	 */
	if (driver_data)
		pr_info("File with api %u found!\n", driver_data->api);

	complete(&test_dev->request_complete);

	return 0;
}

static int trigger_config_async(struct driver_data_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;
	int ret;
	const struct driver_data_req_params req_params_default = {
		DRIVER_DATA_DEFAULT_ASYNC(config_async_req_cb, test_dev),
		.reqs = (config->optional ? DRIVER_DATA_REQ_OPTIONAL : 0) |
			(config->keep ? DRIVER_DATA_REQ_KEEP : 0),
	};
	const struct driver_data_req_params req_params_opt_cb = {
		DRIVER_DATA_DEFAULT_ASYNC(config_async_req_cb, test_dev),
		DRIVER_DATA_ASYNC_OPT_CB(config_async_req_default_cb, test_dev),
		.reqs = (config->optional ? DRIVER_DATA_REQ_OPTIONAL : 0) |
			(config->keep ? DRIVER_DATA_REQ_KEEP : 0),
	};
	const struct driver_data_req_params req_params_api = {
		DRIVER_DATA_API_CB(config_async_req_api_cb, test_dev),
		DRIVER_DATA_API(config->api_min, config->api_max, config->api_name_postfix),
		.reqs = (config->optional ? DRIVER_DATA_REQ_OPTIONAL : 0) |
			(config->keep ? DRIVER_DATA_REQ_KEEP : 0) |
			(config->use_api_versioning ? DRIVER_DATA_REQ_USE_API_VERSIONING : 0),
	};
	const struct driver_data_req_params *req_params;

	if (config->enable_opt_cb)
		req_params = &req_params_opt_cb;
	else if (config->use_api_versioning)
		req_params = &req_params_api;
	else
		req_params = &req_params_default;

	test_dev->api_found_calls = 0;
	ret = driver_data_request_async(config->name, req_params,
					test_dev->dev);
	if (ret) {
		dev_err(test_dev->dev, "async load of '%s' failed: %d\n",
			config->name, ret);
		goto out;
	}

	/*
	 * Without waiting for completion we'd return before the async callback
	 * completes, and any testing analysis done on the results would be
	 * bogus. We could have used async cookies to avoid having drivers
	 * avoid adding their own completions and initializing them.
	 * We have decided its best to keep with the old way of doing things to
	 * keep things compatible. Deal with it.
	 */
	wait_for_completion_timeout(&test_dev->request_complete, 5 * HZ);

out:
	return ret;
}

static ssize_t
trigger_config_store(struct device *dev,
		     struct device_attribute *attr,
		     const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_driver_data_private *test_driver_data =
		&test_dev->test_driver_data;
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->trigger_mutex);
	mutex_lock(&test_dev->config_mutex);

	dev_info(dev, "loading '%s'\n", config->name);

	if (config->async)
		ret = trigger_config_async(test_dev);
	else
		ret = trigger_config_sync(test_dev);

	config->test_result = ret;

	if (ret)
		goto out;

	if (test_driver_data->written) {
		dev_info(dev, "loaded: %zu\n", test_driver_data->size);
		ret = count;
	} else {
		dev_err(dev, "failed to load firmware\n");
		ret = -ENODEV;
	}

out:
	mutex_unlock(&test_dev->config_mutex);
	mutex_unlock(&test_dev->trigger_mutex);

	return ret;
}
static DEVICE_ATTR_WO(trigger_config);

/*
 * XXX: move to kstrncpy() once merged.
 *
 * Users should use kfree_const() when freeing these.
 */
static int __kstrncpy(char **dst, const char *name, size_t count, gfp_t gfp)
{
	*dst = kstrndup(name, count, gfp);
	if (!*dst)
		return -ENOSPC;
	return count;
}

static void __driver_data_config_free(struct test_config *config)
{
	kfree_const(config->name);
	config->name = NULL;
	kfree_const(config->default_name);
	config->default_name = NULL;
	kfree_const(config->api_name_postfix);
	config->api_name_postfix = NULL;
}

static void driver_data_config_free(struct driver_data_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;

	mutex_lock(&test_dev->config_mutex);
	__driver_data_config_free(config);
	mutex_unlock(&test_dev->config_mutex);
}

static int __driver_data_config_init(struct test_config *config)
{
	int ret;

	ret = __kstrncpy(&config->name, TEST_DRIVER_DATA,
			 strlen(TEST_DRIVER_DATA), GFP_KERNEL);
	if (ret < 0)
		goto out;

	ret = __kstrncpy(&config->default_name, TEST_DRIVER_DATA,
			 strlen(TEST_DRIVER_DATA), GFP_KERNEL);
	if (ret < 0)
		goto out;

	config->async = false;
	config->optional = false;
	config->keep = false;
	config->enable_opt_cb = false;
	config->use_api_versioning = false;
	config->api_min = 0;
	config->api_max = 0;
	config->test_result = 0;

	return 0;

out:
	__driver_data_config_free(config);
	return ret;
}

int driver_data_config_init(struct driver_data_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->config_mutex);
	ret = __driver_data_config_init(config);
	mutex_unlock(&test_dev->config_mutex);

	return ret;
}

static ssize_t config_name_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->config_mutex);
	kfree_const(config->name);
	ret = __kstrncpy(&config->name, buf, count, GFP_KERNEL);
	mutex_unlock(&test_dev->config_mutex);

	return ret;
}

/*
 * As per sysfs_kf_seq_show() the buf is max PAGE_SIZE.
 */
static ssize_t config_test_show_str(struct mutex *config_mutex,
				    char *dst,
				    char *src)
{
	int len;

	mutex_lock(config_mutex);
	len = snprintf(dst, PAGE_SIZE, "%s\n", src);
	mutex_unlock(config_mutex);

	return len;
}

static ssize_t config_name_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return config_test_show_str(&test_dev->config_mutex, buf,
				    config->name);
}
static DEVICE_ATTR(config_name, 0644, config_name_show, config_name_store);

static ssize_t config_default_name_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->config_mutex);
	kfree_const(config->default_name);
	ret = __kstrncpy(&config->default_name, buf, count, GFP_KERNEL);
	mutex_unlock(&test_dev->config_mutex);

	return ret;
}

static ssize_t config_default_name_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return config_test_show_str(&test_dev->config_mutex, buf,
				    config->default_name);
}
static DEVICE_ATTR(config_default_name, 0644, config_default_name_show,
		   config_default_name_store);

static ssize_t reset_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->trigger_mutex);

	mutex_lock(&test_dev->driver_data_mutex);
	free_test_driver_data(&test_dev->test_driver_data);
	reinit_completion(&test_dev->request_complete);
	mutex_unlock(&test_dev->driver_data_mutex);

	mutex_lock(&test_dev->config_mutex);

	__driver_data_config_free(config);

	ret = __driver_data_config_init(config);
	if (ret < 0) {
		ret = -ENOMEM;
		dev_err(dev, "could not alloc settings for config trigger: %d\n",
		       ret);
		goto out;
	}

	dev_info(dev, "reset\n");
	ret = count;

out:
	mutex_unlock(&test_dev->config_mutex);
	mutex_unlock(&test_dev->trigger_mutex);

	return ret;
}
static DEVICE_ATTR_WO(reset);

/*
 * XXX: consider a soluton to generalize drivers to specify their own
 * mutex, adding it to dev core after this gets merged. This may not
 * be important for once-in-a-while system tuning parameters, but if
 * we want to enable fuzz testing, this is really important.
 *
 * It may make sense to just have a "struct device configuration mutex"
 * for these sorts of things, although there is difficulty in that we'd
 * need dynamically allocated attributes for that. Its the same reason
 * why we ended up not using the provided standard device attribute
 * bool, int interfaces.
 */

static int test_dev_config_update_bool(struct driver_data_test_device *test_dev,
				       const char *buf, size_t size,
				       bool *config)
{
	int ret;

	mutex_lock(&test_dev->config_mutex);
	if (strtobool(buf, config) < 0)
		ret = -EINVAL;
	else
		ret = size;
	mutex_unlock(&test_dev->config_mutex);

	return ret;
}

static ssize_t
test_dev_config_show_bool(struct driver_data_test_device *test_dev,
			  char *buf,
			  bool config)
{
	bool val;

	mutex_lock(&test_dev->config_mutex);
	val = config;
	mutex_unlock(&test_dev->config_mutex);

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static int test_dev_config_update_int(struct driver_data_test_device *test_dev,
				      const char *buf, size_t size,
				      int *config)
{
	int ret;
	long new;

	ret = kstrtol(buf, 10, &new);
	if (ret)
		return ret;

	if (new > INT_MAX || new < INT_MIN)
		return -EINVAL;

	mutex_lock(&test_dev->config_mutex);
	*(int *)config = new;
	mutex_unlock(&test_dev->config_mutex);

	/* Always return full write size even if we didn't consume all */
	return size;
}

static
ssize_t test_dev_config_show_int(struct driver_data_test_device *test_dev,
				 char *buf,
				 int config)
{
	int val;

	mutex_lock(&test_dev->config_mutex);
	val = config;
	mutex_unlock(&test_dev->config_mutex);

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static int test_dev_config_update_u8(struct driver_data_test_device *test_dev,
				     const char *buf, size_t size,
				     u8 *config)
{
	int ret;
	long new;

	ret = kstrtol(buf, 10, &new);
	if (ret)
		return ret;

	if (new > U8_MAX)
		return -EINVAL;

	mutex_lock(&test_dev->config_mutex);
	*(u8 *)config = new;
	mutex_unlock(&test_dev->config_mutex);

	/* Always return full write size even if we didn't consume all */
	return size;
}

static
ssize_t test_dev_config_show_u8(struct driver_data_test_device *test_dev,
				char *buf,
				u8 config)
{
	u8 val;

	mutex_lock(&test_dev->config_mutex);
	val = config;
	mutex_unlock(&test_dev->config_mutex);

	return snprintf(buf, PAGE_SIZE, "%u\n", val);
}


static ssize_t config_async_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_bool(test_dev, buf, count,
					   &config->async);
}

static ssize_t config_async_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_bool(test_dev, buf, config->async);
}
static DEVICE_ATTR(config_async, 0644, config_async_show, config_async_store);

static ssize_t config_optional_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_bool(test_dev, buf, count,
					   &config->optional);
}

static ssize_t config_optional_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_bool(test_dev, buf, config->optional);
}
static DEVICE_ATTR(config_optional, 0644, config_optional_show,
		   config_optional_store);

static ssize_t config_keep_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_bool(test_dev, buf, count,
					   &config->keep);
}

static ssize_t config_keep_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_bool(test_dev, buf, config->keep);
}
static DEVICE_ATTR(config_keep, 0644, config_keep_show, config_keep_store);

static ssize_t config_enable_opt_cb_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_bool(test_dev, buf, count,
					   &config->enable_opt_cb);
}

static ssize_t config_enable_opt_cb_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_bool(test_dev, buf,
					 config->enable_opt_cb);
}
static DEVICE_ATTR(config_enable_opt_cb, 0644,
		   config_enable_opt_cb_show,
		   config_enable_opt_cb_store);

static ssize_t config_use_api_versioning_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_bool(test_dev, buf, count,
					   &config->use_api_versioning);
}

static ssize_t config_use_api_versioning_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_bool(test_dev, buf,
					 config->use_api_versioning);
}
static DEVICE_ATTR(config_use_api_versioning, 0644,
		   config_use_api_versioning_show,
		   config_use_api_versioning_store);

static ssize_t config_api_min_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_u8(test_dev, buf, count,
					 &config->api_min);
}

static ssize_t config_api_min_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_u8(test_dev, buf, config->api_min);
}
static DEVICE_ATTR(config_api_min, 0644, config_api_min_show, config_api_min_store);

static ssize_t config_api_max_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_u8(test_dev, buf, count,
					 &config->api_max);
}

static ssize_t config_api_max_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_u8(test_dev, buf, config->api_max);
}
static DEVICE_ATTR(config_api_max, 0644, config_api_max_show, config_api_max_store);

static ssize_t config_api_name_postfix_store(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->config_mutex);
	kfree_const(config->api_name_postfix);
	ret = __kstrncpy(&config->api_name_postfix, buf, count, GFP_KERNEL);
	mutex_unlock(&test_dev->config_mutex);

	return ret;
}

static ssize_t config_api_name_postfix_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return config_test_show_str(&test_dev->config_mutex, buf,
				    config->api_name_postfix);
}
static DEVICE_ATTR(config_api_name_postfix, 0644, config_api_name_postfix_show,
		   config_api_name_postfix_store);

static ssize_t test_result_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_int(test_dev, buf, count,
					  &config->test_result);
}

static ssize_t test_result_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct driver_data_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_int(test_dev, buf, config->test_result);
}
static DEVICE_ATTR(test_result, 0644, test_result_show, test_result_store);

#define TEST_DRIVER_DATA_DEV_ATTR(name)		&dev_attr_##name.attr

static struct attribute *test_dev_attrs[] = {
	TEST_DRIVER_DATA_DEV_ATTR(trigger_config),
	TEST_DRIVER_DATA_DEV_ATTR(config),
	TEST_DRIVER_DATA_DEV_ATTR(reset),

	TEST_DRIVER_DATA_DEV_ATTR(config_name),
	TEST_DRIVER_DATA_DEV_ATTR(config_default_name),
	TEST_DRIVER_DATA_DEV_ATTR(config_async),
	TEST_DRIVER_DATA_DEV_ATTR(config_optional),
	TEST_DRIVER_DATA_DEV_ATTR(config_keep),
	TEST_DRIVER_DATA_DEV_ATTR(config_use_api_versioning),
	TEST_DRIVER_DATA_DEV_ATTR(config_enable_opt_cb),
	TEST_DRIVER_DATA_DEV_ATTR(config_api_min),
	TEST_DRIVER_DATA_DEV_ATTR(config_api_max),
	TEST_DRIVER_DATA_DEV_ATTR(config_api_name_postfix),
	TEST_DRIVER_DATA_DEV_ATTR(test_result),

	NULL,
};

ATTRIBUTE_GROUPS(test_dev);

void free_test_dev_driver_data(struct driver_data_test_device *test_dev)
{
	kfree_const(test_dev->misc_dev.name);
	test_dev->misc_dev.name = NULL;
	vfree(test_dev);
	test_dev = NULL;
	driver_data_config_free(test_dev);
}

void unregister_test_dev_driver_data(struct driver_data_test_device *test_dev)
{
	wait_for_completion_timeout(&test_dev->request_complete, 5 * HZ);
	dev_info(test_dev->dev, "removing interface\n");
	misc_deregister(&test_dev->misc_dev);
	kfree(&test_dev->misc_dev.name);
	free_test_dev_driver_data(test_dev);
}

struct driver_data_test_device *alloc_test_dev_driver_data(int idx)
{
	int ret;
	struct driver_data_test_device *test_dev;
	struct miscdevice *misc_dev;

	test_dev = vzalloc(sizeof(struct driver_data_test_device));
	if (!test_dev)
		goto err_out;

	mutex_init(&test_dev->driver_data_mutex);
	mutex_init(&test_dev->config_mutex);
	mutex_init(&test_dev->trigger_mutex);
	init_completion(&test_dev->request_complete);

	ret = driver_data_config_init(test_dev);
	if (ret < 0)
		goto err_out_free;

	test_dev->dev_idx = idx;
	misc_dev = &test_dev->misc_dev;

	misc_dev->minor = MISC_DYNAMIC_MINOR;
	misc_dev->name = kasprintf(GFP_KERNEL, "test_driver_data%d", idx);
	if (!misc_dev->name)
		goto err_out_free_config;

	misc_dev->fops = &test_fw_fops;
	misc_dev->groups = test_dev_groups;

	return test_dev;

err_out_free_config:
	__driver_data_config_free(&test_dev->config);
err_out_free:
	kfree(test_dev);
err_out:
	return NULL;
}

static int register_test_dev_driver_data(void)
{
	struct driver_data_test_device *test_dev = NULL;
	int ret = -ENODEV;

	mutex_lock(&reg_dev_mutex);

	/* int should suffice for number of devices, test for wrap */
	if (unlikely(num_test_devs + 1) < 0) {
		pr_err("reached limit of number of test devices\n");
		goto out;
	}

	test_dev = alloc_test_dev_driver_data(num_test_devs);
	if (!test_dev) {
		ret = -ENOMEM;
		goto out;
	}

	ret = misc_register(&test_dev->misc_dev);
	if (ret) {
		pr_err("could not register misc device: %d\n", ret);
		free_test_dev_driver_data(test_dev);
		goto out;
	}

	test_dev->dev = test_dev->misc_dev.this_device;
	list_add_tail(&test_dev->list, &reg_test_devs);
	dev_info(test_dev->dev, "interface ready\n");

	num_test_devs++;

out:
	mutex_unlock(&reg_dev_mutex);

	return ret;
}

static int __init test_driver_data_init(void)
{
	int ret;

	ret = register_test_dev_driver_data();
	if (ret)
		pr_err("Cannot add first test driver_data device\n");

	return ret;
}
late_initcall(test_driver_data_init);

static void __exit test_driver_data_exit(void)
{
	struct driver_data_test_device *test_dev, *tmp;

	mutex_lock(&reg_dev_mutex);
	list_for_each_entry_safe(test_dev, tmp, &reg_test_devs, list) {
		list_del(&test_dev->list);
		unregister_test_dev_driver_data(test_dev);
	}
	mutex_unlock(&reg_dev_mutex);
}

module_exit(test_driver_data_exit);

MODULE_AUTHOR("Luis R. Rodriguez <mcgrof@kernel.org>");
MODULE_LICENSE("GPL");
