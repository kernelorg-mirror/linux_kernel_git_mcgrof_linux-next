====================
Linux section ranges
====================

This documents Linux' use of section ranges, how you can use
them and how they work.

About section ranges
====================

Introduction
------------
.. kernel-doc:: include/linux/ranges.h
   :doc: Introduction

Section range module support
----------------------------
.. kernel-doc:: include/linux/ranges.h
   :doc: Section range module support

Section range helpers
=====================
.. kernel-doc:: include/linux/ranges.h
   :doc: Section range helpers

SECTION_RANGE_START
-------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: SECTION_RANGE_START

SECTION_RANGE_END
-----------------
.. kernel-doc:: include/linux/ranges.h
   :functions: SECTION_RANGE_END

SECTION_RANGE_SIZE
------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: SECTION_RANGE_SIZE

SECTION_RANGE_EMPTY
-------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: SECTION_RANGE_EMPTY

SECTION_RANGE_ADDR_WITHIN
-------------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: SECTION_RANGE_ADDR_WITHIN

SECTION_RANGE_ALIGNMENT
-------------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: SECTION_RANGE_ALIGNMENT

DECLARE_SECTION_RANGE
---------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: DECLARE_SECTION_RANGE

DEFINE_SECTION_RANGE
--------------------
.. kernel-doc:: include/linux/ranges.h
   :functions: DEFINE_SECTION_RANGE

__LINUX_RANGE
-------------
.. kernel-doc:: include/asm-generic/ranges.h
   :functions: __LINUX_RANGE

__LINUX_RANGE_ORDER
-------------------
.. kernel-doc:: include/asm-generic/ranges.h
   :functions: __LINUX_RANGE_ORDER
