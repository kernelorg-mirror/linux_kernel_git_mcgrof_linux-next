#!/bin/bash
# Copyright (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of copyleft-next (version 0.3.1 or later) as published
# at http://copyleft-next.org/.

# This performs a series tests against firmware_class to excercise the
# firmware_class driver with focus only on the extensible driver data API.
#
# To make this test self contained, and not pollute your distribution
# firmware install paths, we reset the custom load directory to a
# temporary location.

set -e

TEST_NAME="driver_data"
TEST_DRIVER="test_${TEST_NAME}"
TEST_DIR=$(dirname $0)

# This represents
#
# TEST_ID:TEST_COUNT:ENABLED
#
# TEST_ID: is the test id number
# TEST_COUNT: number of times we should run the test
# ENABLED: 1 if enabled, 0 otherwise
#
# Once these are enabled please leave them as-is. Write your own test,
# we have tons of space.
ALL_TESTS="0001:3:1"
ALL_TESTS="$ALL_TESTS 0002:3:1"
ALL_TESTS="$ALL_TESTS 0003:3:1"
ALL_TESTS="$ALL_TESTS 0004:10:1"
ALL_TESTS="$ALL_TESTS 0005:10:1"
ALL_TESTS="$ALL_TESTS 0006:10:1"
ALL_TESTS="$ALL_TESTS 0007:10:1"
ALL_TESTS="$ALL_TESTS 0008:10:1"
ALL_TESTS="$ALL_TESTS 0009:10:1"
ALL_TESTS="$ALL_TESTS 0010:10:1"
ALL_TESTS="$ALL_TESTS 0011:10:1"
ALL_TESTS="$ALL_TESTS 0012:1:1"
ALL_TESTS="$ALL_TESTS 0013:1:1"

# Not yet sure how to automate suspend test well yet.  For now we expect a
# manual run. If using qemu you can resume a guest using something like the
# following on the monitor pts.
# system_wakeupakeup | socat - /dev/pts/7,raw,echo=0,crnl
#ALL_TESTS="$ALL_TESTS 0014:0:1"

test_modprobe()
{
       if [ ! -d $DIR ]; then
               echo "$0: $DIR not present" >&2
               echo "You must have the following enabled in your kernel:" >&2
               cat $TEST_DIR/config >&2
               exit 1
       fi
}

function allow_user_defaults()
{
	if [ -z $DEFAULT_NUM_TESTS ]; then
		DEFAULT_NUM_TESTS=50
	fi

	if [ -z $FW_SYSFSPATH ]; then
		FW_SYSFSPATH="/sys/module/firmware_class/parameters/path"
	fi

	if [ -z $OLD_FWPATH ]; then
		OLD_FWPATH=$(cat $FW_SYSFSPATH)
	fi

	if [ -z $FWPATH]; then
		FWPATH=$(mktemp -d)
	fi

	if [ -z $DEFAULT_DRIVER_DATA ]; then
		config_reset
		DEFAULT_DRIVER_DATA=$(config_get_name)
	fi

	if [ -z $FW ]; then
		FW="$FWPATH/$DEFAULT_DRIVER_DATA"
	fi

	if [ -z $SYS_STATE_PATH ]; then
		SYS_STATE_PATH="/sys/power/state"
	fi

	# Set the kernel search path.
	echo -n "$FWPATH" > $FW_SYSFSPATH

	# This is an unlikely real-world firmware content. :)
	echo "ABCD0123" >"$FW"
}

test_reqs()
{
	if ! which diff 2> /dev/null > /dev/null; then
		echo "$0: You need diff installed"
		exit 1
	fi

	uid=$(id -u)
	if [ $uid -ne 0 ]; then
		echo $msg must be run as root >&2
		exit 0
	fi
}

function load_req_mod()
{
	trap "test_modprobe" EXIT

	if [ -z $DIR ]; then
		DIR="/sys/devices/virtual/misc/${TEST_DRIVER}0/"
	fi

	if [ ! -d $DIR ]; then
		modprobe $TEST_DRIVER
	fi
}

test_finish()
{
	echo -n "$OLD_PATH" >/sys/module/firmware_class/parameters/path
	rm -f "$FW"
	rmdir "$FWPATH"
}

errno_name_to_val()
{
	case "$1" in
	SUCCESS)
		echo 0;;
	-EPERM)
		echo -1;;
	-ENOENT)
		echo -2;;
	-EINVAL)
		echo -22;;
	-ERR_ANY)
		echo -123456;;
	*)
		echo invalid;;
	esac
}

errno_val_to_name()
	case "$1" in
	0)
		echo SUCCESS;;
	-1)
		echo -EPERM;;
	-2)
		echo -ENOENT;;
	-22)
		echo -EINVAL;;
	-123456)
		echo -ERR_ANY;;
	*)
		echo invalid;;
	esac

config_set_async()
{
	if ! echo -n 1 >$DIR/config_async ; then
		echo "$0: Unable to set to async" >&2
		exit 1
	fi
}

config_disable_async()
{
	if ! echo -n 0 >$DIR/config_async ; then
		echo "$0: Unable to set to sync" >&2
		exit 1
	fi
}

config_set_optional()
{
	if ! echo -n 1 >$DIR/config_optional ; then
		echo "$0: Unable to set to optional" >&2
		exit 1
	fi
}

config_disable_optional()
{
	if ! echo -n 0 >$DIR/config_optional ; then
		echo "$0: Unable to disable optional" >&2
		exit 1
	fi
}

config_set_keep()
{
	if ! echo -n 1 >$DIR/config_keep; then
		echo "$0: Unable to set to keep" >&2
		exit 1
	fi
}

config_disable_keep()
{
	if ! echo -n 0 >$DIR/config_keep; then
		echo "$0: Unable to disable keep option" >&2
		exit 1
	fi
}

config_enable_opt_cb()
{
	if ! echo -n 1 >$DIR/config_enable_opt_cb; then
		echo "$0: Unable to set to optional" >&2
		exit 1
	fi
}

config_enable_api_versioning()
{
	if ! echo -n 1 >$DIR/config_use_api_versioning; then
		echo "$0: Unable to set use_api_versioning option" >&2
		exit 1
	fi
}

config_set_api_name_postfix()
{
	if ! echo -n $1 >$DIR/config_api_name_postfix; then
		echo "$0: Unable to set use_api_versioning option" >&2
		exit 1
	fi
}

config_set_api_min()
{
	if ! echo -n $1 >$DIR/config_api_min; then
		echo "$0: Unable to set config_api_min option" >&2
		exit 1
	fi
}

config_set_api_max()
{
	if ! echo -n $1 >$DIR/config_api_max; then
		echo "$0: Unable to set config_api_max option" >&2
		exit 1
	fi
}

config_add_api_file()
{
	TMP_FW="$FWPATH/$1"
	echo "ABCD0123" >"$TMP_FW"
}

config_rm_api_file()
{
	TMP_FW="$FWPATH/$1"
	rm -f $TMP_FW
}

# For special characters use printf directly,
# refer to driver_data_test_0001
config_set_name()
{
	if ! echo -n $1 >$DIR/config_name; then
		echo "$0: Unable to set name" >&2
		exit 1
	fi
}

config_get_name()
{
	cat $DIR/config_name
}

# For special characters use printf directly,
# refer to driver_data_test_0001
config_set_default_name()
{
	if ! echo -n $1 >$DIR/config_default_name; then
		echo "$0: Unable to set default_name" >&2
		exit 1
	fi
}

config_get_default_name()
{
	cat $DIR/config_default_name
}

config_get_test_result()
{
	cat $DIR/test_result
}

config_reset()
{
	if ! echo -n "1" >"$DIR"/reset; then
		echo "$0: reset shuld have worked" >&2
		exit 1
	fi
}

trigger_release_driver_data()
{
	if ! echo -n "1" >"$DIR"/trigger_release_driver_data; then
		echo "$0: release driver data shuld have worked" >&2
		exit 1
	fi
}

config_show_config()
{
	echo "----------------------------------------------------"
	cat "$DIR"/config
	echo "----------------------------------------------------"
}

config_trigger()
{
	if ! echo -n "1" >"$DIR"/trigger_config 2>/dev/null; then
		echo "$1: FAIL - loading should have worked" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! - loading driver_data"
}

config_trigger_want_fail()
{
	if echo "1" > $DIR/trigger_config 2>/dev/null; then
		echo "$1: FAIL - loading was expected to fail" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! - loading failed as expected"
}

config_file_should_match()
{
	FILE=$(config_get_name)
	if [ ! -z $2 ]; then
		FILE=$2
	fi
	# On this one we expect the file to exist so leave stderr in
	if ! $(diff -q "$FWPATH"/"$FILE" /dev/test_driver_data0 > /dev/null) > /dev/null; then
		echo "$1: FAIL - file $FILE did not match contents in /dev/test_driver_data0" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! - $FILE == /dev/test_driver_data0"
}

config_file_should_match_default()
{
	FILE=$(config_get_default_name)
	# On this one we expect the file to exist so leave stderr in
	if ! $(diff -q "$FWPATH"/"$FILE" /dev/test_driver_data0 > /dev/null) > /dev/null; then
		echo "$1: FAIL - file $FILE did not match contents in /dev/test_driver_data0" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! - $FILE == /dev/test_driver_data0"
}

config_file_should_not_match()
{
	FILE=$(config_get_name)
	# File may not exist, so skip those error messages as well
	if $(diff -q $FWPATH/$FILE /dev/test_driver_data0 2> /dev/null) 2> /dev/null ; then
		echo "$1: FAIL - file $FILE was not expected to match /dev/null" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! - $FILE != /dev/test_driver_data0"
}

config_default_file_should_match()
{
	FILE=$(config_get_default_name)
	diff -q $FWPATH/$FILE /dev/test_driver_data0 2> /dev/null
	if ! $? ; then
		echo "$1: FAIL - file $FILE expected to match /dev/test_driver_data0" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! [file integrity matches]"
}

config_default_file_should_not_match()
{
	FILE=$(config_get_default_name)
	diff -q FWPATH/$FILE /dev/test_driver_data0 2> /dev/null
	if $? 2> /dev/null ; then
		echo "$1: FAIL - file $FILE was not expected to match test_driver_data0" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK!"
}

config_expect_result()
{
	RC=$(config_get_test_result)
	RC_NAME=$(errno_val_to_name $RC)

	ERRNO_NAME=$2
	ERRNO=$(errno_name_to_val $ERRNO_NAME)

	if [[ $ERRNO_NAME = "-ERR_ANY" ]]; then
		if [[ $RC -ge 0 ]]; then
			echo "$1: FAIL, test expects $ERRNO_NAME - got $RC_NAME ($RC)" >&2
			config_show_config >&2
			exit 1
		fi
	elif [[ $RC != $ERRNO ]]; then
		echo "$1: FAIL, test expects $ERRNO_NAME ($ERRNO) - got $RC_NAME ($RC)" >&2
		config_show_config >&2
		exit 1
	fi
	echo "$1: OK! - Return value: $RC ($RC_NAME), expected $ERRNO_NAME"
}

driver_data_set_sync_defaults()
{
	config_reset
}

driver_data_set_async_defaults()
{
	config_reset
	config_set_async
}

set_system_state()
{
	STATE="mem"
	if [ ! -z $2 ]; then
		STATE=$2
	fi
	echo $STATE > $SYS_STATE_PATH
}

driver_data_test_0001s()
{
	NAME='\000'

	driver_data_set_sync_defaults
	config_set_name $NAME
	printf '\000' >"$DIR"/config_name
	config_trigger_want_fail ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} -EINVAL
}

driver_data_test_0001a()
{
	NAME='\000'

	driver_data_set_async_defaults
	printf '\000' >"$DIR"/config_name
	config_trigger_want_fail ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} -EINVAL
}

driver_data_test_0001()
{
	driver_data_test_0001s
	driver_data_test_0001a
}

driver_data_test_0002s()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_sync_defaults
	config_set_name $NAME
	config_trigger_want_fail ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} -ENOENT
}

driver_data_test_0002a()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_async_defaults
	config_set_name $NAME
	config_trigger_want_fail ${FUNCNAME[0]}
	# This may seem odd to expect success on a bogus
	# file but remember this is an async call, the actual
	# error handling is managed by the async callbacks.
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0002()
{
	driver_data_test_0002s
	driver_data_test_0002a
}

driver_data_test_0003()
{
	config_reset
	config_file_should_not_match ${FUNCNAME[0]}
}

driver_data_test_0004s()
{
	driver_data_set_sync_defaults
	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0004a()
{
	driver_data_set_async_defaults
	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0004()
{
	driver_data_test_0004s
	driver_data_test_0004a
}

driver_data_test_0005s()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_sync_defaults
	config_set_optional
	config_set_name $NAME
	config_trigger_want_fail ${FUNCNAME[0]}
	# We do this to ensure the default backup callback hasn't
	# been called yet
	config_file_should_not_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} -ENOENT
}

driver_data_test_0005a()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_async_defaults
	config_set_optional
	config_set_name $NAME
	config_trigger_want_fail ${FUNCNAME[0]}
	# We do this to ensure the default backup callback hasn't
	# been called yet
	config_file_should_not_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0005()
{
	driver_data_test_0005s
	driver_data_test_0005a
}

driver_data_test_0006s()
{
	driver_data_set_sync_defaults
	config_set_optional
	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0006a()
{
	driver_data_set_async_defaults
	config_set_optional
	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0006()
{
	driver_data_test_0006s
	driver_data_test_0006a
}

driver_data_test_0007s()
{
	driver_data_set_sync_defaults
	config_set_keep
	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0007a()
{
	driver_data_set_async_defaults
	config_set_keep
	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0007()
{
	driver_data_test_0007s
	driver_data_test_0007a
}

driver_data_test_0008s()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_sync_defaults
	config_set_name $NAME
	config_set_optional
	config_enable_opt_cb
	config_trigger ${FUNCNAME[0]}
	config_file_should_match_default ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0008a()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_async_defaults
	config_set_name $NAME
	config_set_optional
	config_enable_opt_cb
	config_trigger ${FUNCNAME[0]}
	config_file_should_match_default ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0008()
{
	driver_data_test_0008s
	driver_data_test_0008a
}

driver_data_test_0009s()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_sync_defaults
	config_set_name $NAME
	config_set_keep
	config_set_optional
	config_enable_opt_cb
	config_trigger ${FUNCNAME[0]}
	config_file_should_match_default ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0009a()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_async_defaults
	config_set_name $NAME
	config_set_keep
	config_set_optional
	config_enable_opt_cb
	config_trigger ${FUNCNAME[0]}
	config_file_should_match_default ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0009()
{
	driver_data_test_0009s
	driver_data_test_0009a
}

driver_data_test_0010s()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_sync_defaults
	config_set_name $NAME
	config_set_default_name $NAME
	config_set_keep
	config_set_optional
	config_enable_opt_cb
	config_trigger_want_fail ${FUNCNAME[0]}
	config_file_should_not_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} -ENOENT
}

driver_data_test_0010a()
{
	NAME="nope-$DEFAULT_DRIVER_DATA"

	driver_data_set_async_defaults
	config_set_name $NAME
	config_set_default_name $NAME
	config_set_keep
	config_set_optional
	config_enable_opt_cb
	config_trigger_want_fail ${FUNCNAME[0]}
	config_file_should_not_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0010()
{
	driver_data_test_0010s
	driver_data_test_0010a
}

driver_data_test_0011a()
{
	driver_data_set_async_defaults
	config_set_keep
	config_enable_api_versioning

	config_trigger_want_fail ${FUNCNAME[0]}
	config_file_should_not_match ${FUNCNAME[0]}
	config_expect_result ${FUNCNAME[0]} -EINVAL
}

driver_data_test_0011()
{
	driver_data_test_0011a
}

driver_data_test_0012a()
{
	driver_data_set_async_defaults
	NAME_PREFIX="driver_data_test_0012a_"
	TARGET_API="4"
	NAME_POSTFIX=".bin"
	NAME="${NAME_PREFIX}${TARGET_API}${NAME_POSTFIX}"

	config_set_name $NAME_PREFIX
	config_set_keep
	config_enable_api_versioning
	config_set_api_name_postfix ".bin"
	config_set_api_min 3
	config_set_api_max 18

	config_trigger_want_fail ${FUNCNAME[0]}
	config_file_should_not_match ${FUNCNAME[0]} $NAME
	config_expect_result ${FUNCNAME[0]} SUCCESS
}

driver_data_test_0012()
{
	driver_data_test_0012a
}

driver_data_test_0013a()
{
	driver_data_set_async_defaults
	NAME_PREFIX="driver_data_test_0013a_"
	TARGET_API="4"
	NAME_POSTFIX=".bin"
	NAME="${NAME_PREFIX}${TARGET_API}${NAME_POSTFIX}"

	config_set_name $NAME_PREFIX
	config_set_keep
	config_enable_api_versioning
	config_set_api_name_postfix $NAME_POSTFIX
	config_set_api_min 3
	config_set_api_max 18
	config_add_api_file $NAME

	config_trigger ${FUNCNAME[0]}
	config_file_should_match ${FUNCNAME[0]} $NAME
	config_expect_result ${FUNCNAME[0]} SUCCESS
	config_rm_api_file $NAME
}

driver_data_test_0013()
{
	driver_data_test_0013a
}

driver_data_test_0014a()
{
	driver_data_set_async_defaults
	NAME_PREFIX="driver_data_test_0013a_"
	TARGET_API="4"
	NAME_POSTFIX=".bin"
	NAME="${NAME_PREFIX}${TARGET_API}${NAME_POSTFIX}"

	config_set_name $NAME_PREFIX
	config_set_keep
	config_enable_api_versioning
	config_set_api_name_postfix $NAME_POSTFIX
	config_set_api_min 3
	config_set_api_max 18
	config_add_api_file $NAME

	config_trigger ${FUNCNAME[0]}

	# suspend to memory
	set_system_state mem

	config_file_should_match ${FUNCNAME[0]} $NAME
	config_expect_result ${FUNCNAME[0]} SUCCESS
	config_rm_api_file $NAME
}

driver_data_test_0014()
{
	driver_data_test_0014a
}

list_tests()
{
	echo "Test ID list:"
	echo
	echo "TEST_ID x NUM_TEST"
	echo "TEST_ID:   Test ID"
	echo "NUM_TESTS: Number of recommended times to run the test"
	echo
	echo "0001 x $(get_test_count 0001) - Empty string should be ignored"
	echo "0002 x $(get_test_count 0002) - Files that do not exist should be ignored"
	echo "0003 x $(get_test_count 0003) - Verify test_driver_data0 has nothing loaded upon reset"
	echo "0004 x $(get_test_count 0004) - Simple sync and async loader"
	echo "0005 x $(get_test_count 0005) - Verify optional loading is not fatal"
	echo "0006 x $(get_test_count 0006) - Verify optional loading enables loading"
	echo "0007 x $(get_test_count 0007) - Verify keep works"
	echo "0008 x $(get_test_count 0008) - Verify optional callback works"
	echo "0009 x $(get_test_count 0009) - Verify optional callback works, keep"
	echo "0010 x $(get_test_count 0010) - Verify when fallback file is not present"
	echo "0011 x $(get_test_count 0011) - Verify api setup will fail on invalid values"
	echo "0012 x $(get_test_count 0012) - Verify api call wills will hunt for files, ignore file"
	echo "0013 x $(get_test_count 0013) - Verify api call works"
	echo "0014 x $(get_test_count 0013) - Verify api call works with suspend + resume"
}

test_reqs

usage()
{
	NUM_TESTS=$(grep -o ' ' <<<"$ALL_TESTS" | grep -c .)
	let NUM_TESTS=$NUM_TESTS+1
	MAX_TEST=$(printf "%04d\n" $NUM_TESTS)
	echo "Usage: $0 [ -t <4-number-digit> ] | [ -w <4-number-digit> ] |"
	echo "		 [ -s <4-number-digit> ] | [ -c <4-number-digit> <test-count>"
	echo "           [ all ] [ -h | --help ] [ -l ]"
	echo ""
	echo "Valid tests: 0001-$MAX_TEST"
	echo ""
	echo "    all     Runs all tests (default)"
	echo "    -t      Run test ID the number amount of times is recommended"
	echo "    -w      Watch test ID run until it runs into an error"
	echo "    -s      Run test ID once"
	echo "    -c      Run test ID x test-count number of times"
	echo "    -l      List all test ID list"
	echo " -h|--help  Help"
	echo
	echo "If an error every occurs execution will immediately terminate."
	echo "If you are adding a new test try using -w <test-ID> first to"
	echo "make sure the test passes a series of tests."
	echo
	echo Example uses:
	echo
	echo "$TEST_NAME.sh            -- executes all tests"
	echo "$TEST_NAME.sh -t 0008    -- Executes test ID 0008 number of times is recomended"
	echo "$TEST_NAME.sh -w 0008    -- Watch test ID 0008 run until an error occurs"
	echo "$TEST_NAME.sh -s 0008    -- Run test ID 0008 once"
	echo "$TEST_NAME.sh -c 0008 3  -- Run test ID 0008 three times"
	echo
	list_tests
	exit 1
}

function test_num()
{
	re='^[0-9]+$'
	if ! [[ $1 =~ $re ]]; then
		usage
	fi
}

function get_test_count()
{
	test_num $1
	TEST_DATA=$(echo $ALL_TESTS | awk '{print $'$1'}')
	LAST_TWO=${TEST_DATA#*:*}
	echo ${LAST_TWO%:*}
}

function get_test_enabled()
{
	test_num $1
	TEST_DATA=$(echo $ALL_TESTS | awk '{print $'$1'}')
	echo ${TEST_DATA#*:*:}
}

function run_all_tests()
{
	for i in $ALL_TESTS ; do
		TEST_ID=${i%:*:*}
		ENABLED=$(get_test_enabled $TEST_ID)
		TEST_COUNT=$(get_test_count $TEST_ID)
		if [[ $ENABLED -eq "1" ]]; then
			test_case $TEST_ID $TEST_COUNT
		fi
	done
}

function watch_log()
{
	if [ $# -ne 3 ]; then
		clear
	fi
	date
	echo "Running test: $2 - run #$1"
}

function watch_case()
{
	i=0
	while [ 1 ]; do

		if [ $# -eq 1 ]; then
			test_num $1
			watch_log $i ${TEST_NAME}_test_$1
			${TEST_NAME}_test_$1
		else
			watch_log $i all
			run_all_tests
		fi
		let i=$i+1
	done
}

function test_case()
{
	NUM_TESTS=$DEFAULT_NUM_TESTS
	if [ $# -eq 2 ]; then
		NUM_TESTS=$2
	fi

	i=0
	while [ $i -lt $NUM_TESTS ]; do
		test_num $1
		watch_log $i ${TEST_NAME}_test_$1 noclear
		RUN_TEST=${TEST_NAME}_test_$1
		$RUN_TEST
		let i=$i+1
	done
}

function parse_args()
{
	if [ $# -eq 0 ]; then
		run_all_tests
	else
		if [[ "$1" = "all" ]]; then
			run_all_tests
		elif [[ "$1" = "-w" ]]; then
			shift
			watch_case $@
		elif [[ "$1" = "-t" ]]; then
			shift
			test_num $1
			test_case $1 $(get_test_count $1)
		elif [[ "$1" = "-c" ]]; then
			shift
			test_num $1
			test_num $2
			test_case $1 $2
		elif [[ "$1" = "-s" ]]; then
			shift
			test_case $1 1
		elif [[ "$1" = "-l" ]]; then
			list_tests
		elif [[ "$1" = "-h" || "$1" = "--help" ]]; then
			usage
		else
			usage
		fi
	fi
}

test_reqs
load_req_mod
allow_user_defaults

trap "test_finish" EXIT

parse_args $@

exit 0
