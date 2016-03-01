===============
driver_data API
===============

The driver data APIs provides a flexible API for general driver data file
lookups. Its flexibility aims at mitigating collateral evolutions on the kernel
as new functionality is introduced.

Driver data modes of operation
==============================

There are two types of modes of operation for driver data requests:

  * synchronous  - driver_data_request_sync()
  * asynchronous - driver_data_request_async()

Synchronous requests expect requests to be done immediately, asynchronous
requests enable requests to be scheduled for a later time.

Driver data request parameters
==============================

Variations of types of driver data requests are specified by a driver data
request parameter data structure. The flexibility of the API is provided by
expanding the request parameters as new functionality is needed, without
loosely modifying or adding new exported APIs.

Reference counting and releasing the driver data file
=====================================================

The device and module are bumped with reference counts during the driver data
requests. This prevents removal of the device and module making the driver data
request call until the driver data request callbacks have completed, either
synchronously or asynchronously. When synchronous requests are made the
firmware_class is refcounted. When asynchronous requests are made the caller's
module is refcounted. Asynchronous requests do not refcount the firmware_class
module.

The driver data request API enables callers to provide a callback for both
synchronous and asynchronous requests and since consumption can be expected
in these callbacks it frees it for you by default after callback handlers
are issued. If you wish to keep the driver data around after your callbacks
you must specify this through the driver data request parameter data structure.

Testing the driver_data API
===========================

The driver data API has a selftest driver: lib/test_driver_data.c. The
test_driver_data enables you to build your tests in userspace by exposing knobs
of the exported API in userspace and enabling userspace to configure and
trigger a kernel call. This lets us build most possible test cases of
the kernel APIs from userspace.

The test_driver_data also enables multiple test triggers to be created
enabling testing to be done in parallel, one test interface per test case.

To test an async call one could do::

        echo anything > /lib/firmware/test-driver_data.bin
        echo -n 1 >  /sys/devices/virtual/misc/test_driver_data0/config_async
        echo -n 1 >  /sys/devices/virtual/misc/test_driver_data0/trigger_config

A series of tests have been written to test the driver data API thoroughly.
A respective test case is expected to bet written as new features get added.
For details of existing tests run::

        tools/testing/selftests/firmware/driver_data.sh -l

To see all available options::

        tools/testing/selftests/firmware/driver_data.sh --help

To run a test 0010 case 40 times::

        tools/testing/selftests/firmware/driver_data.sh -c 0010 40

Note that driver_data.sh uses its own temporary custom path for creating and
looking for driver data files, it does this to not overwrite any production
files you might have which may share the same names used by the test shell
script driver_data.sh. If you are not using the driver_data.sh script your
default path will be used.

Tracking development enhancements and ideas
===========================================

To help track ongoing development for firmware_class and related items to
firmware_class refer to the kernel newbies wiki page [0].

[0] http://kernelnewbies.org/KernelProjects/firmware-class-enhancements
