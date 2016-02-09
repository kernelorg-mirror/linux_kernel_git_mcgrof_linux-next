==============================
Core Linux kernel ELF sections
==============================

About
=====

This book documents the different standard and custom ELF sections used
on the Linux kernel, which we refer to as the ``core Linux sections``. We
start off by documenting the standard ELF sections used by Linux and move
on to the basic custom ELF sections, followed by a set of helpers. Each
section documented describes the goal of the section, and addresses
concurrency considerations when applicable.

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Custom linker script

Standard ELF section use in Linux
=================================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Standard ELF section use in Linux

SECTION_RODATA
--------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_RODATA

SECTION_RODATA
--------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_TEXT

SECTION_DATA
------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_DATA

Linux .init\* sections
======================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux init sections

SECTION_INIT_DATA
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_INIT_DATA

SECTION_INIT_RODATA
-------------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_INIT_RODATA

SECTION_INIT_CALL
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_INIT_CALL

Linux .exit\* sections
======================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux exit sections

SECTION_EXIT
------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_EXIT

SECTION_EXIT_DATA
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_EXIT_DATA

SECTION_EXIT_CALL
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_EXIT_CALL

Linux .ref\* sections
=====================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux references to init sections

SECTION_REF
-----------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_REF

SECTION_REF_DATA
----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_REF_DATA

SECTION_REF_RODATA
------------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_REF_RODATA

Linux section ordering
======================
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux section ordering

SECTION_ORDER_ANY
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: SECTION_ORDER_ANY

Generic Linux kernel section helpers
====================================

Introduction
-------------
.. kernel-doc:: include/linux/sections.h
   :doc: Introduction

LINUX_SECTION_ALIGNMENT
-----------------------
.. kernel-doc:: include/linux/sections.h
   :functions: LINUX_SECTION_ALIGNMENT

LINUX_SECTION_SIZE
------------------
.. kernel-doc:: include/linux/sections.h
   :functions: LINUX_SECTION_SIZE

LINUX_SECTION_EMPTY
-------------------
.. kernel-doc:: include/linux/sections.h
   :functions: LINUX_SECTION_EMPTY

LINUX_SECTION_START
-------------------
.. kernel-doc:: include/linux/sections.h
   :functions: LINUX_SECTION_START

LINUX_SECTION_END
-----------------
.. kernel-doc:: include/linux/sections.h
   :functions: LINUX_SECTION_END

DECLARE_LINUX_SECTION
---------------------
.. kernel-doc:: include/linux/sections.h
   :functions: DECLARE_LINUX_SECTION

DECLARE_LINUX_SECTION_RO
------------------------
.. kernel-doc:: include/linux/sections.h
   :functions: DECLARE_LINUX_SECTION_RO
