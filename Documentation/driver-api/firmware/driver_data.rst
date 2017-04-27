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

Tracking development enhancements and ideas
===========================================

To help track ongoing development for firmware_class and related items to
firmware_class refer to the kernel newbies wiki page [0].

[0] http://kernelnewbies.org/KernelProjects/firmware-class-enhancements
