#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Test loading built-in firmware
set -e

TEST_REQS_FW_SYSFS_FALLBACK="no"
TEST_REQS_FW_SET_CUSTOM_PATH="yes"
TEST_DIR=$(dirname $0)
source $TEST_DIR/fw_lib.sh

echo -n "Testing builtin firmware requesting... "
echo 1 >  $DIR/reset
echo -n $1 > $DIR/config_name
echo -n 1 > $DIR/trigger_request_builtin
echo "OK"
