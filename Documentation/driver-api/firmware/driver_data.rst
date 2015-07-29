===============
driver_data API
===============

Users of firmware request APIs has grown to include users which are not
looking for "firmware", but instead general driver data files which have
been kept oustide of the kernel. The driver data APIs addresses rebranding
of firmware as generic driver data files, and provides a flexible API which
mitigates collateral evolutions on the kernel as new functionality is
introduced.

Driver data modes of operation
==============================

There are only two types of modes of operation for driver data requests:

  * synchronous  - driver_data_request()
  * asynchronous - driver_data_request_async()

Synchronous requests expect requests to be done immediately, asynchronous
requests enable requests to be scheduled for a later time.

Driver data request parameters
==============================

Variations of types of driver data requests are specified by a driver data
request parameter data structure. This data structure is expected to grow as
new requirements grow.

Reference counting and releasing the driver data file
=====================================================

As with the old firmware API both the device and module are bumped with
reference counts during the driver data requests. This prevents removal
of the device and module making the driver data request call until the
driver data request callbacks have completed, either synchronously or
asynchronously.

The old firmware APIs refcounted the firmware_class module for synchronous
requests, meanwhile asynchronous requests refcounted the caller's module.
The driver data request API currently mimics this behaviour, for synchronous
requests the firmware_class module is refcounted through the use of
dfl_sync_reqs. In the future we may enable the ability to also refcount the
caller's module as well. Likewise in the future we may enable asynchronous
calls to refcount the firmware_class module.

Typical use of the old synchronous firmware APIs consist of the caller
requesting for "driver data", consuming it after a request and finally
freeing it. Typical asynchronous use of the old firmware APIs consist of
the caller requesting for "driver data" and then finally freeing it on
asynchronous callback.

The driver data request API enables callers to provide a callback for both
synchronous and asynchronous requests and since consumption can be expected
in these callbacks it frees it for you by default after callback handlers
are issued. If you wish to keep the driver data around after your callbacks
you must specify this through the driver data request parameter data structure.

Synchronizing with async cookies
================================

The driver data API relies on async cookies to enable users to synchronize
for any pending async work. The async cookie obtained through an async
call using driver_data_file_request_async() can be used to synchronize and
wait for pending work with driver_data_synchronize_request().

When driver_data_file_request_async() completes you can rest assured all the
work for both triggering, and processing the driver data using any of your
callbacks has completed.

Tracking development enhancements and ideas
===========================================

To help track ongoing development for firmware_class and related items to
firmware_class refer to the kernel newbies wiki page [0].

[0] http://kernelnewbies.org/KernelProjects/firmware-class-enhancements
