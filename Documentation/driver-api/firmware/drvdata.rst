===========
drvdata API
===========

As the kernel evolves we keep extending the firmware_class set of APIs
with more or less arguments, this creates a slew of collateral evolutions.
The set of users of firmware request APIs has also grown now to include
users which are not looking for "firmware" per se, but instead general
driver data files which for one reason or another has been decided to be
kept oustide of the kernel, and/or to allow dynamic updates. The driver data
request set of APIs addresses rebranding of firmware as generic driver data
files, and provides a way to enable these APIs to easily be extended without
much collateral evolutions.

Driver data modes of operation
==============================

There are only two types of modes of operation for system data requests:

  * synchronous  - drvdata_request()
  * asynchronous - drvdata_request_async()

Synchronous requests expect requests to be done immediately, asynchronous
requests enable requests to be scheduled for a later time.

Driver data request parameters
==============================

Variations of types of driver data requests are specified by a driver data
request parameter data structure. This data structure can grow as with new
fields as requirements grow. The old firmware API provides two synchronous
requests: request_firmware() and request_firmware_direct(), the later allowing
the caller to specify that the "driver data file" is optional.  The driver data
request API allows a caller to set the optional nature of the driver data
on the request parameter data structure using the same synchronous API. Since
this requirement is part of the request paramter data structure it also allows
asynchronous requests to specify that the driver data is optional.

Reference counting and releasing the system data file
=====================================================

As with the old firmware API both the device and module are bumped with
reference counts during the driver data requests. This prevents removal
of the device and module making the driver data request call until the
driver data request callbacks have completed, either synchronously or
asynchronously.

The old firmware APIs refcounted the firmware_class module for synchronous
requests, meanwhile asynchronous requests refcounted the caller's module.
The driver data request API currently mimic this behaviour, for synchronous
requests the firmware_class module is refcounted through the use of
dfl_sync_reqs, although if in the future we may later enable use of
also refcounting the caller's module as well. Likewise in the future we
may extend asynchronous calls to refcount the firmware_class module.

Typical use of the old synchronous firmware APIs consist of the caller
requesting for "driver data", consuming it after a request and finally
freeing it. Typical asynchronous use of the old firmware APIs consist of
the caller requesting for "driver data" and then finally freeing it on
asynchronous callback.

The driver data request API enables callers to provide a callback for both
synchronous and asynchronous requests and since consumption can be expected
in these callbacks it frees it for you by default after callback handlers
are issued. If you wish to keep the driver data around after your callbacks
you must specify this through the driver data request paramter data structure.

Async cookies, replacing completions
====================================

With this new API you do not need to declare and use your own completions, you
can replace your completions with drvdata_synchronize_request() using the
async_cookie set for you by drvdata_file_request_async(). When
drvdata_file_request_async() completes you can rest assured all the work for
both triggering, and processing the drvdata using any of your callbacks has
completed.

Fallback mechanisms on the driver data API
==========================================

The old firmware API provided support for a series of fallback mechanisms. The
new driver data API abandons all current notions of the fallback mechanisms,
it may soon add support for one though.

Tracking development enhancements and ideas
===========================================

To help track ongoing development for firmware_class and related items to
firmware_class refer to the kernel newbies wiki page [0].

[0] http://kernelnewbies.org/KernelProjects/firmware-class-enhancements
