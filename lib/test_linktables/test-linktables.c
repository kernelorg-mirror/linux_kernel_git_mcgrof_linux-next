/*
 * Linker table test driver
 *
 * Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of copyleft-next (version 0.3.1 or later) as published
 * at http://copyleft-next.org/.
 */

/*
 * This driver provides an interface to trigger and test the linker table API
 * through a series of configurations and a few triggers. This driver
 * must be built-in as linker tables currently lack generalized modular
 * support -- additional work is required on each module right now if you
 * want to use linker tables, in the future we may make this much easier.
 * For now just build this into your kernel.
 *
 * echo -n .text > /sys/devices/virtual/misc/test_linktable/config_section_name
 * echo -n 3 > /sys/devices/virtual/misc/test_linktable/config_input
 * echo 1 > /sys/devices/virtual/misc/test_linktable/trigger_config
 * cat /sys/devices/virtual/misc/test_linktable/test_result
 * 30
 *
 * echo -n .data > /sys/devices/virtual/misc/test_linktable/config_section_name
 * echo -n .text > /sys/devices/virtual/misc/test_linktable/config_section_name
 * echo -n .rodata > /sys/devices/virtual/misc/test_linktable/config_section_name
 *
 * To check the configuration:
 *
 * cat /sys/devices/virtual/misc/test_linktable/config
 *
 * NUM_TESTS
 *    ∑        test(n, input)
 *   n=0
 *
 * Each linker table entry on each section has a series of entries. Each entry
 * has a function which just multiplies the test case number by input value.
 *
 * ...
 * test-linktables-03.c multiplies (3 * input)
 * test-linktables-04.c multiplies (4 * input)
 * ...
 *
 * A full run on a section produces the sum of these values. So with input set
 * to 3 we have:
 *
 * (0 * 3) + (1 * 3) + (2 * 3) + (3 * 3) + (4 * 3)
 *    0    +    3    +    6    +   9     +  12
 *                    30
 *
 * This is nothing important, its just a basic test. We had to pick something.
 * Other than the above tests, this also demos and shows proper const use on
 * all sections which we need as read-only: .text, .init.text, .rodata.
 * Furthermore it does a silly write test to ensure write works on .data and
 * init.data. Since the read-only sections use const we obviously are forced
 * by the compiler to not be able to write to these sections.
 *
 * One of the more important items, the order, is tested. We could develop
 * a fancy simple math algorithm that depends on order for correctness but
 * instead we just annotate the expected order as we run. If the order listed
 * on test->expected does not match with the actual order a routine was run
 * in then we fail and complain.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/tables.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "test-linktables.h"

#define NUM_TESTS __LINKTABLE_TESTS

struct linktable_test_device *__test_dev;
static bool init_completed;

static const char *test_dev_name = "test_linktable";

/*
 * Used for the default test to start configuration with if we reset the
 * test from scratch. Don't use .init.* sections given we can only test
 * these on init, when init_completed is true we can't .init.* section
 * code anymore.
 */
#define TEST_SECTION_START ".data"

DEFINE_LINKTABLE(struct test_linktable, test_fns_data);
DEFINE_LINKTABLE_TEXT(struct test_linktable, test_fns_text);
DEFINE_LINKTABLE_RO(struct test_linktable, test_fns_rodata);
DEFINE_LINKTABLE_INIT(struct test_linktable, test_fns_init_text);
DEFINE_LINKTABLE_INIT_DATA(struct test_linktable, test_fns_init_data);

struct test_config {
	char *section_name;
	int input;
	int test_result;
};

/**
 * linktable_test_device - test device to help test linker tables
 *
 * @dev_idx: unique ID for test device
 * @misc_dev: we use a misc device under the hood
 * @dev: pointer to misc_dev's own struct device
 * @config_mutex: protects configuration of test
 * @trigger_mutex: the test trigger can only be fired once at a time
 */
struct linktable_test_device {
	struct test_config config;
	struct miscdevice misc_dev;
	struct device *dev;
	struct mutex config_mutex;
	struct mutex trigger_mutex;
	unsigned int num_called;
};

/**
 * enum linktable_test_case - linker table test case
 *
 * @TEST_LINKTABLE_INIT: tests .init.text (this is all const)
 * @TEST_LINKTABLE_INIT_DATA: tests .init.data
 * @TEST_LINKTABLE: tests .data
 * @TEST_LINKTABLE_TEXT: tests .text (this is all const)
 * @TEST_LINKTABLE_RO: tests .rodata (this is all const)
 */
enum linktable_test_case {
	__TEST_LINKTABLE_INVALID = 0,

	TEST_LINKTABLE_INIT,
	TEST_LINKTABLE_INIT_DATA,
	TEST_LINKTABLE,
	TEST_LINKTABLE_TEXT,
	TEST_LINKTABLE_RO,
};

static struct miscdevice *dev_to_misc_dev(struct device *dev)
{
	return dev_get_drvdata(dev);
}

static
struct linktable_test_device *misc_dev_to_test_dev(struct miscdevice *misc_dev)
{
	return container_of(misc_dev, struct linktable_test_device, misc_dev);
}

static struct linktable_test_device *dev_to_test_dev(struct device *dev)
{
	struct miscdevice *misc_dev;

	misc_dev = dev_to_misc_dev(dev);

	return misc_dev_to_test_dev(misc_dev);
}

static ssize_t config_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int len = 0;

	mutex_lock(&test_dev->config_mutex);

	len += snprintf(buf, PAGE_SIZE,
			"Custom trigger configuration for: %s\n",
			dev_name(dev));

	if (config->section_name)
		len += snprintf(buf+len, PAGE_SIZE,
				"section name:\t%s\n",
				config->section_name);
	else
		len += snprintf(buf+len, PAGE_SIZE,
				"default name:\tEMTPY\n");

	mutex_unlock(&test_dev->config_mutex);

	return len;
}
static DEVICE_ATTR_RO(config);

static int write_test(struct test_linktable *test, int write_val)
{
	test->write_test = write_val;
	if (test->write_test != write_val) {
		pr_warn("Write test failed\n");
		return -EPERM;
	}
	return 0;
}

static int __test_linktable(struct linktable_test_device *test_dev,
			    struct test_linktable *test)
{
	struct test_config *config = &test_dev->config;
	int rc;

	if (test->expected != test_dev->num_called) {
		pr_warn("test routine ID %d called on order %d\n",
			test->expected, test_dev->num_called);
		return -EINVAL;
	}

	rc = write_test(test, test->expected+1);
	if (rc)
		return rc;

	rc = test->op(config->input);
	test_dev->num_called++;

	return rc;
}

static int __test_linktable_ro(struct linktable_test_device *test_dev,
			       const struct test_linktable *test)
{
	struct test_config *config = &test_dev->config;
	int rc;

	if (test->expected != test_dev->num_called) {
		pr_info("test routine ID %d called on order %d\n",
			test->expected, test_dev->num_called);
		return -EINVAL;
	}

	/*
	 * Note, compiler would complain if we tried write_test() so
	 * no need to test that.
	 */

	rc = test->op(config->input);
	test_dev->num_called++;

	return rc;
}

static enum linktable_test_case get_test_case(const char *section_name)
{
	if (strcmp(".init.text", section_name) == 0)
		return TEST_LINKTABLE_INIT;
	if (strcmp(".init.data", section_name) == 0)
		return TEST_LINKTABLE_INIT_DATA;
	if (strcmp(".data", section_name) == 0)
		return TEST_LINKTABLE;
	if (strcmp(".text", section_name) == 0)
		return TEST_LINKTABLE_TEXT;
	if (strcmp(".rodata", section_name) == 0)
		return TEST_LINKTABLE_TEXT;

	return __TEST_LINKTABLE_INVALID;
}

static int __run_sanity_test(struct linktable_test_device *test_dev,
			     int num_entries)
{
	if (!num_entries) {
		pr_warn("no tests -- this is invalid\n");
		return -EINVAL;
	}

	pr_debug("number of tests: %d\n", num_entries);

	if (num_entries != NUM_TESTS) {
		pr_warn("expected: %d test\n", NUM_TESTS);
		return -EINVAL;
	}

	return 0;
}

static int __init run_init_text_test(struct linktable_test_device *test_dev)
{
	int ret, total = 0;
	const struct test_linktable *test;
	unsigned int num_entries = LINKTABLE_SIZE(test_fns_init_text);

	ret = __run_sanity_test(test_dev, num_entries);
	if (ret)
		return ret;

	linktable_for_each(test, test_fns_init_text)
		total += __test_linktable_ro(test_dev, test);

	return total;
}

static int __init run_init_data_test(struct linktable_test_device *test_dev)
{
	int ret, total = 0;
	struct test_linktable *test;
	unsigned int num_entries = LINKTABLE_SIZE(test_fns_init_data);

	ret = __run_sanity_test(test_dev, num_entries);
	if (ret)
		return ret;

	linktable_for_each(test, test_fns_init_data)
		total += __test_linktable(test_dev, test);

	return total;
}

static int run_data_test(struct linktable_test_device *test_dev)
{
	int ret, total = 0;
	struct test_linktable *test;
	unsigned int num_entries = LINKTABLE_SIZE(test_fns_data);

	ret = __run_sanity_test(test_dev, num_entries);
	if (ret)
		return ret;

	linktable_for_each(test, test_fns_data)
		total += __test_linktable(test_dev, test);

	return total;
}

static int run_text_test(struct linktable_test_device *test_dev)
{
	int ret, total = 0;
	const struct test_linktable *test;
	unsigned int num_entries = LINKTABLE_SIZE(test_fns_text);

	ret = __run_sanity_test(test_dev, num_entries);
	if (ret)
		return ret;

	linktable_for_each(test, test_fns_text)
		total += __test_linktable_ro(test_dev, test);

	return total;
}

static int run_rodata_test(struct linktable_test_device *test_dev)
{
	int ret, total = 0;
	const struct test_linktable *test;
	unsigned int num_entries = LINKTABLE_SIZE(test_fns_rodata);

	ret = __run_sanity_test(test_dev, num_entries);
	if (ret)
		return ret;

	linktable_for_each(test, test_fns_rodata)
		total += __test_linktable_ro(test_dev, test);

	return total;
}

static int __ref __trigger_config_run(struct linktable_test_device *test_dev)
{
	enum linktable_test_case test_case;
	struct test_config *config = &test_dev->config;

	test_dev->num_called = 0;
	test_case = get_test_case(config->section_name);

	switch (test_case) {
	case TEST_LINKTABLE_INIT:
		if (!init_completed)
			return run_init_text_test(test_dev);
		else
			return -EACCES;
	case TEST_LINKTABLE_INIT_DATA:
		if (!init_completed)
			return run_init_data_test(test_dev);
		else
			return -EACCES;
	case TEST_LINKTABLE:
		return run_data_test(test_dev);
	case TEST_LINKTABLE_TEXT:
		return run_text_test(test_dev);
	case TEST_LINKTABLE_RO:
		return run_rodata_test(test_dev);
	default:
		pr_warn("Invalid test case requested: %s\n",
			config->section_name);
		return -EINVAL;
	}
}

static int trigger_config_run(struct linktable_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;
	int rc;

	mutex_lock(&test_dev->trigger_mutex);
	mutex_lock(&test_dev->config_mutex);

	pr_debug("running test on '%s'\n", config->section_name);

	rc = __trigger_config_run(test_dev);

	config->test_result = rc;
	pr_debug("result: %d\n", rc);

	if (rc < 0) {
		rc = -EINVAL;
		goto out;
	}

	rc = 0;

out:
	mutex_unlock(&test_dev->config_mutex);
	mutex_unlock(&test_dev->trigger_mutex);

	return rc;
}

static ssize_t
trigger_config_store(struct device *dev,
		     struct device_attribute *attr,
		     const char *buf, size_t count)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	int rc;

	rc = trigger_config_run(test_dev);
	if (rc)
		goto out;

	rc = count;
out:
	return rc;
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

static int config_copy_section_name(struct test_config *config,
				    const char *name,
				    size_t count)
{
	return __kstrncpy(&config->section_name, name, count, GFP_KERNEL);
}

static void __linktable_config_free(struct test_config *config)
{
	kfree_const(config->section_name);
	config->section_name = NULL;
}

static void linktable_config_free(struct linktable_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;

	mutex_lock(&test_dev->config_mutex);
	__linktable_config_free(config);
	mutex_unlock(&test_dev->config_mutex);
}

static ssize_t config_section_name_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int rc;

	mutex_lock(&test_dev->config_mutex);
	rc = config_copy_section_name(config, buf, count);
	mutex_unlock(&test_dev->config_mutex);

	return rc;
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

static ssize_t config_section_name_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return config_test_show_str(&test_dev->config_mutex, buf,
				    config->section_name);
}
static DEVICE_ATTR(config_section_name, 0644, config_section_name_show,
		   config_section_name_store);

static int trigger_config_run_named(struct linktable_test_device *test_dev,
				    const char *section_name)
{
	int copied;
	struct test_config *config = &test_dev->config;

	mutex_lock(&test_dev->config_mutex);
	copied = config_copy_section_name(config, section_name,
					  strlen(section_name));
	mutex_unlock(&test_dev->config_mutex);

	if (copied <= 0 || copied != strlen(section_name))
		return -EINVAL;

	return trigger_config_run(test_dev);
}

static int __linktable_config_init(struct test_config *config)
{
	int ret;

	ret = config_copy_section_name(config, TEST_SECTION_START,
				       strlen(TEST_SECTION_START));
	if (ret < 0)
		goto out;

	config->input = 3;
	config->test_result = 0;

out:
	return ret;
}

static ssize_t reset_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->trigger_mutex);
	mutex_lock(&test_dev->config_mutex);

	__linktable_config_free(config);

	ret = __linktable_config_init(config);
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

static int test_dev_config_update_int(struct linktable_test_device *test_dev,
				      const char *buf, size_t size,
				      int *config)
{
	long new;
	int ret;

	ret = kstrtol(buf, 10, &new);
	if (ret)
		return ret;

	if (new > INT_MAX || new < INT_MIN)
		return -EINVAL;

	mutex_lock(&test_dev->config_mutex);
	*config = new;
	mutex_unlock(&test_dev->config_mutex);
	/* Always return full write size even if we didn't consume all */
	return size;
}

static ssize_t test_dev_config_show_int(struct linktable_test_device *test_dev,
					char *buf,
					int config)
{
	int val;

	mutex_lock(&test_dev->config_mutex);
	val = config;
	mutex_unlock(&test_dev->config_mutex);

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t test_result_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_int(test_dev, buf, count,
					  &config->test_result);
}

static ssize_t config_input_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_update_int(test_dev, buf, count,
					  &config->input);
}

static ssize_t config_input_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_int(test_dev, buf, config->input);
}
static DEVICE_ATTR(config_input, 0644, config_input_show, config_input_store);

static ssize_t test_result_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct linktable_test_device *test_dev = dev_to_test_dev(dev);
	struct test_config *config = &test_dev->config;

	return test_dev_config_show_int(test_dev, buf, config->test_result);
}
static DEVICE_ATTR(test_result, 0644, test_result_show, test_result_store);

#define LINKTABLE_DEV_ATTR(name)		&dev_attr_##name.attr

static struct attribute *test_dev_attrs[] = {
	LINKTABLE_DEV_ATTR(trigger_config),
	LINKTABLE_DEV_ATTR(config),
	LINKTABLE_DEV_ATTR(reset),

	LINKTABLE_DEV_ATTR(config_section_name),
	LINKTABLE_DEV_ATTR(config_input),
	LINKTABLE_DEV_ATTR(test_result),

	NULL,
};

ATTRIBUTE_GROUPS(test_dev);

static int linktable_config_init(struct linktable_test_device *test_dev)
{
	struct test_config *config = &test_dev->config;
	int ret;

	mutex_lock(&test_dev->config_mutex);
	ret = __linktable_config_init(config);
	mutex_unlock(&test_dev->config_mutex);

	return ret;
}

static struct linktable_test_device *alloc_test_dev_linktable(void)
{
	int rc;
	struct linktable_test_device *test_dev;
	struct miscdevice *misc_dev;

	test_dev = vmalloc(sizeof(struct linktable_test_device));
	if (!test_dev)
		goto err_out;

	memset(test_dev, 0, sizeof(struct linktable_test_device));

	mutex_init(&test_dev->config_mutex);
	mutex_init(&test_dev->trigger_mutex);

	rc = linktable_config_init(test_dev);
	if (rc < 0) {
		pr_err("Cannot alloc linktable_config_init()\n");
		goto err_out_free;
	}

	misc_dev = &test_dev->misc_dev;

	misc_dev->minor = MISC_DYNAMIC_MINOR;
	misc_dev->name = test_dev_name;
	misc_dev->groups = test_dev_groups;

	return test_dev;

err_out_free:
	kfree(test_dev);
err_out:
	return NULL;
}

static void free_test_dev_linktable(struct linktable_test_device *test_dev)
{
	test_dev->misc_dev.name = NULL;
	vfree(test_dev);
	test_dev = NULL;
	linktable_config_free(test_dev);
}

static struct linktable_test_device *register_test_dev_linktable(void)
{
	struct linktable_test_device *test_dev = NULL;
	int rc;

	test_dev = alloc_test_dev_linktable();
	if (!test_dev)
		return NULL;

	rc = misc_register(&test_dev->misc_dev);
	if (rc) {
		pr_err("could not register misc device: %d\n", rc);
		free_test_dev_linktable(test_dev);
		return NULL;
	}

	test_dev->dev = test_dev->misc_dev.this_device;
	dev_dbg(test_dev->dev, "interface ready\n");

	return test_dev;
}

static int __init test_linktable_init(void)
{
	struct linktable_test_device *test_dev;
	int rc;

	test_dev = register_test_dev_linktable();
	if (!test_dev) {
		pr_err("Cannot add test linktable device\n");
		return -ENODEV;
	}

	rc = trigger_config_run_named(test_dev, ".init.text");
	if (WARN_ON(rc))
		return rc;
	rc = trigger_config_run_named(test_dev, ".init.data");
	if (WARN_ON(rc))
		return rc;

	init_completed = true;

	rc = trigger_config_run_named(test_dev, ".data");
	if (WARN_ON(rc))
		return rc;
	rc = trigger_config_run_named(test_dev, ".text");
	if (WARN_ON(rc))
		return rc;
	rc = trigger_config_run_named(test_dev, ".rodata");
	if (WARN_ON(rc))
		return rc;

	pr_info("linker table tests: OK!\n");

	return 0;
}
late_initcall(test_linktable_init);

static
void unregister_test_dev_linktable(struct linktable_test_device *test_dev)
{
	dev_info(test_dev->dev, "removing interface\n");
	misc_deregister(&test_dev->misc_dev);
	free_test_dev_linktable(test_dev);
}

static void __exit test_linktable_exit(void)
{
	struct linktable_test_device *test_dev = __test_dev;

	unregister_test_dev_linktable(test_dev);
}
module_exit(test_linktable_exit);

MODULE_AUTHOR("Luis R. Rodriguez <mcgrof@kernel.org>");
MODULE_LICENSE("GPL");
