============
Introduction
============

The firmware API enables kernel code to request files required
for functionality from userspace, the uses vary:

* Microcode for CPU errata
* Device driver firmware, required to be loaded onto device
  microcontrollers
* Device driver information data (calibration data, EEPROM overrides),
  some of which can be completely optional.

Types of firmware requests
==========================

There are two types of calls:

* Synchronous
* Asynchronous

Which one you use vary depending on your requirements, the rule of thumb
however is you should strive to use the asynchronous APIs unless you also
are already using asynchronous initialization mechanisms which will not
stall or delay boot. Even if loading firmware does not take a lot of time
processing firmware might, and this can still delay boot or initialization,
as such mechanisms such as asynchronous probe can help supplement drivers.

Two APIs
========

Two APIs are provided for firmware:

* Old firmware API - :ref:`request_firmware`
* Flexible driver data API - :ref:`driver_data`

We have historically extended the firmware API by adding new routines or at
times extending existing routines with more or less arguments. This doesn't
scale well, when new arguments are added to existing routines it means we need
to traverse the kernel with a slew of collateral evolutions to adjust old
driver users.  The driver data API is an extensible API enabling extensions to
be added by avoiding unnecessary collateral evolutions as features get added.
New features and development should be added through the driver_data API.
