#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Test loading built-in firmware
set -e

TEST_REQS_FW_SYSFS_FALLBACK="no"
TEST_REQS_FW_SET_CUSTOM_PATH="yes"
TEST_DIR=$(dirname $0)
source $TEST_DIR/fw_lib.sh

check_mods
check_setup
verify_reqs
setup_tmp_file

trap "test_finish" EXIT

if [ "$HAS_FW_LOADER_USER_HELPER" = "yes" ]; then
	# Turn down the timeout so failures don't take so long.
	echo 1 >/sys/class/firmware/timeout
fi

# built-in firmware support can be optional to test
if [[ "$HAS_FW_LOADER_BUILTIN" != "yes" || "$HAS_TEST_FIRMWARE_BUILTIN" != "yes" ]]; then
	exit $ksft_skip
fi

MICROCODE_BUILTIN_AMD_DIR="amd-ucode"
if [[ -f $PROC_CONFIG && "$(kconfig_has CONFIG_MICROCODE_BUILTIN_AMD=y)" == "yes" ]]; then
	MICROCODE_BUILTIN_AMD=$(zgrep ^CONFIG_MICROCODE_BUILTIN_AMD_DIR $PROC_CONFIG | awk -F"=" '{print $2}')
fi

echo "Testing builtin firmware API ... "

config_trigger_builtin()
{
	echo -n 1 > $DIR/trigger_request_builtin
}

test_builtin_firmware()
{
	echo -n "Testing firmware_request_builtin() ... "
	config_reset
	config_set_name $TEST_FIRMWARE_BUILTIN_FILENAME
	config_trigger_builtin
	echo OK
	# Verify the contents are what we expect.
	echo -n "Verifying file integrity ..."
	if ! diff -q "$FW" /dev/test_firmware >/dev/null ; then
		echo "$0: firmware loaded content differs" >&2
		exit 1
	else
		echo "firmware content matches what we expect - OK"
	fi
}

test_builtin_firmware_nofile()
{
	echo -n "Testing firmware_request_builtin() with fake file... "
	config_reset
	config_set_name fake-${TEST_FIRMWARE_BUILTIN_FILENAME}
	if config_trigger_builtin 2> /dev/null; then
		echo "$0: firmware shouldn't have loaded" >&2
	fi
	echo "OK"
}

test_builtin_firmware_microcode()
{
	MICROCODE="$1"
	MICROCODE_MD5SUM="$2"
	CONFIG_GREP="$3"

	echo -n "Testing firmware_request_builtin() with $MICROCODE ..."
	config_reset
	if [[ "$(kconfig_has CONFIG_MICROCODE_BUILTIN_AMD_10H_14H=y)" != "yes" ]]; then
		echo "Skipping as its not built-in"
	fi
	config_set_name $MICROCODE
	config_trigger_builtin
	echo OK

	# Verify the contents are what we expect.
	echo -n "Verifying file md5sum ... "

	BUILTIN_MD5SUM=$(md5sum /dev/test_firmware | awk '{print $1}')
	if [[ "$MICROCODE_MD5SUM" != "$BUILTIN_MD5SUM" ]]; then
		echo "failed, expected and got the following" >&2
		echo "$MICROCODE_MD5SUM" >&2
		echo "$BUILTIN_MD5SUM" >&2
		exit 1
	else
		echo "$BUILTIN_MD5SUM OK!"
	fi
}

test_builtin_firmware_amd_10h_14h()
{
	MY_MICROCODE="$MICROCODE_BUILTIN_AMD_DIR/microcode_amd.bin"
	MY_MICROCODE_MD5SUM="55ae79b82cbfddcf7142058be3c9ec2d"
	MY_CONFIG_SYMBOL="CONFIG_MICROCODE_BUILTIN_AMD_10H_14H"

	test_builtin_firmware_microcode $MY_MICROCODE $MY_MICROCODE_MD5SUM $MY_CONFIG_SYMBOL
}

test_builtin_firmware_amd_15h()
{
	MY_MICROCODE="$MICROCODE_BUILTIN_AMD_DIR/microcode_amd_fam15h.bin"
	MY_MICROCODE_MD5SUM="3bdedb4466186a79c469f62120f6d7bb"
	MY_CONFIG_SYMBOL="CONFIG_MICROCODE_BUILTIN_AMD_15H"

	test_builtin_firmware_microcode $MY_MICROCODE $MY_MICROCODE_MD5SUM $MY_CONFIG_SYMBOL
}

test_builtin_firmware_amd_16h()
{
	MY_MICROCODE="$MICROCODE_BUILTIN_AMD_DIR/microcode_amd_fam16h.bin"
	MY_MICROCODE_MD5SUM="6a47a6393c52ddfc0b5b044efc076a77"
	MY_CONFIG_SYMBOL="CONFIG_MICROCODE_BUILTIN_AMD_16H"

	test_builtin_firmware_microcode $MY_MICROCODE $MY_MICROCODE_MD5SUM $MY_CONFIG_SYMBOL
}

test_builtin_firmware_amd_17h()
{
	MY_MICROCODE="$MICROCODE_BUILTIN_AMD_DIR/microcode_amd_fam17h.bin"
	MY_MICROCODE_MD5SUM="60f18b6d7fa3d1231b27cc339c173c8c"
	MY_CONFIG_SYMBOL="CONFIG_MICROCODE_BUILTIN_AMD_17H"

	test_builtin_firmware_microcode $MY_MICROCODE $MY_MICROCODE_MD5SUM $MY_CONFIG_SYMBOL
}

test_builtin_firmware
test_builtin_firmware_nofile

test_builtin_firmware_amd_10h_14h
test_builtin_firmware_amd_15h
test_builtin_firmware_amd_16h
test_builtin_firmware_amd_17h

# Ensure test_fw_config->is_builtin is set back to false
# otherwise we won't be able to diff against the right target
# firmware for other tests.
config_reset
