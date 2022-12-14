// SPDX-License-Identifier: GPL-2.0-only
//Copyright(c) 2021 Intel Corporation. All rights reserved.

#include <linux/export.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/srcu.h>
#include "mock.h"

static LIST_HEAD(mock);
DEFINE_SRCU(cxl_mock_srcu);

struct cxl_mock_ops *get_cxl_mock_ops(int *index)
{
	*index = srcu_read_lock(&cxl_mock_srcu);
	return list_first_or_null_rcu(&mock, struct cxl_mock_ops, list);
}
EXPORT_SYMBOL_GPL(get_cxl_mock_ops);

void put_cxl_mock_ops(int index)
{
	srcu_read_unlock(&cxl_mock_srcu, index);
}
EXPORT_SYMBOL_GPL(put_cxl_mock_ops);

void register_cxl_mock_ops(struct cxl_mock_ops *ops)
{
	list_add_rcu(&ops->list, &mock);
}
EXPORT_SYMBOL_GPL(register_cxl_mock_ops);

void unregister_cxl_mock_ops(struct cxl_mock_ops *ops)
{
	list_del_rcu(&ops->list);
	synchronize_srcu(&cxl_mock_srcu);
}
EXPORT_SYMBOL_GPL(unregister_cxl_mock_ops);
