===================
Linux linker tables
===================

This documents Linux linker tables, it explains what they are, where they
came from, how they work, the benefits of using them and more importantly
how you can use them.

About Linker tables
===================
.. kernel-doc:: include/linux/tables.h
   :doc: Introduction

Linker table provenance
---------------------------------------------

.. kernel-doc:: include/linux/tables.h
   :doc: Linker table provenance

Benefits of using Linker tables
===============================

Avoids modifying architecture linker scripts
----------------------------------------------
.. kernel-doc:: include/linux/tables.h
   :doc: Avoids modifying architecture linker scripts

How linker tables simplify initialization code
----------------------------------------------
.. kernel-doc:: include/linux/tables.h
   :doc: How linker tables simplify initialization code

The code bit-rot problem
------------------------
.. kernel-doc:: include/linux/tables.h
   :doc: The code bit-rot problem

The build-all selective-link philosophy
---------------------------------------
.. kernel-doc:: include/linux/tables.h
   :doc: The build-all selective-link philosophy

Avoiding the code bit-rot problem with linker tables
----------------------------------------------------
.. kernel-doc:: include/linux/tables.h
   :doc: Avoiding the code bit-rot problem with linker tables

Using linker tables in Linux
============================

Linker table module support
---------------------------

.. kernel-doc:: include/linux/tables.h
   :doc: Linker table module support

Linker table helpers
====================

.. kernel-doc:: include/linux/tables.h
   :doc: Linker table helpers

LINKTABLE_ADDR_WITHIN
---------------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_ADDR_WITHIN

Constructing linker tables
==========================

.. kernel-doc:: include/linux/tables.h
   :doc: Constructing linker tables

Weak linker tables constructors
-------------------------------

.. kernel-doc:: include/linux/tables.h
   :doc: Weak linker tables constructors

LINKTABLE_WEAK
--------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_WEAK

LINKTABLE_TEXT_WEAK
-------------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_TEXT_WEAK

LINKTABLE_RO_WEAK
-----------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_RO_WEAK

LINKTABLE_INIT_WEAK
-------------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_INIT_WEAK

LINKTABLE_INIT_DATA_WEAK
------------------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_INIT_DATA_WEAK

Regular linker linker table constructors
----------------------------------------

.. kernel-doc:: include/linux/tables.h
   :doc: Regular linker linker table constructors

LINKTABLE
---------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE

LINKTABLE_TEXT
--------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_TEXT

LINKTABLE_RO
------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_RO

LINKTABLE_INIT
--------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_INIT

LINKTABLE_INIT_DATA
-------------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_INIT_DATA

Declaring Linker tables
=======================

.. kernel-doc:: include/linux/tables.h
   :doc: Declaring Linker tables

DECLARE_LINKTABLE
----------------------
.. kernel-doc:: include/linux/tables.h
   :functions: DECLARE_LINKTABLE

DECLARE_LINKTABLE_RO
--------------------
.. kernel-doc:: include/linux/tables.h
   :functions: DECLARE_LINKTABLE_RO

Defining Linker tables
======================

.. kernel-doc:: include/linux/tables.h
   :doc: Defining Linker tables

DEFINE_LINKTABLE
----------------
.. kernel-doc:: include/linux/tables.h
   :functions: DEFINE_LINKTABLE

DEFINE_LINKTABLE_TEXT
---------------------
.. kernel-doc:: include/linux/tables.h
   :functions: DEFINE_LINKTABLE_TEXT

DEFINE_LINKTABLE_RO
-------------------
.. kernel-doc:: include/linux/tables.h
   :functions: DEFINE_LINKTABLE_RO

DEFINE_LINKTABLE_INIT
---------------------
.. kernel-doc:: include/linux/tables.h
   :functions: DEFINE_LINKTABLE_INIT

DEFINE_LINKTABLE_INIT_DATA
--------------------------
.. kernel-doc:: include/linux/tables.h
   :functions: DEFINE_LINKTABLE_INIT_DATA

Iterating over Linker tables
============================

.. kernel-doc:: include/linux/tables.h
   :doc: Iterating over Linker tables

LINKTABLE_FOR_EACH
------------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_FOR_EACH

LINKTABLE_RUN_ALL
-----------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_RUN_ALL

LINKTABLE_RUN_ERR
-----------------
.. kernel-doc:: include/linux/tables.h
   :functions: LINKTABLE_RUN_ERR
