#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <cxl.h>
#include "mock.h"

struct acpi_device *mock_to_cxl_host_bridge(struct device *host,
					    struct device *dev)
{
	int index;
	struct acpi_device *found = NULL;
	struct cxl_mock_ops *ops = get_cxl_mock_ops(&index);

	if (ops && ops->is_mock_bridge(dev)) {
		found = ACPI_COMPANION(dev);
		goto out;
	}

	if (dev->bus == &platform_bus_type)
		goto out;

	found = __to_cxl_host_bridge(host, dev);
out:
	put_cxl_mock_ops(index);
	return found;
}
EXPORT_SYMBOL_NS_GPL(mock_to_cxl_host_bridge, CXL);

MODULE_IMPORT_NS(CXL);
