// SPDX-License-Identifier: GPL-2.0-only
/* Copyright(c) 2020 Intel Corporation. All rights reserved. */
#include <linux/io-64-nonatomic-lo-hi.h>
#include <linux/memregion.h>
#include <linux/workqueue.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/acpi.h>
#include <cxlmem.h>
#include <cxlpci.h>
#include <cxl.h>
#include "core.h"

struct acpi_device *__to_cxl_host_bridge(struct device *host,
					 struct device *dev)
{
	struct acpi_device *adev = to_acpi_device(dev);

	if (!acpi_pci_find_root(adev->handle))
		return NULL;

	if (strcmp(acpi_device_hid(adev), "ACPI0016") == 0)
		return adev;
	return NULL;
}
EXPORT_SYMBOL_NS_GPL(__to_cxl_host_bridge, CXL);
