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

Memory protection
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Memory protection

mark_rodata_ro
-----------------------
.. kernel-doc:: include/linux/init.h
   :functions: mark_rodata_ro

.rodata
-------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .rodata

.text
-----
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .text

.data
------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .data

Linux .init\* sections
======================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux init sections

.init.text
----------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .init.text

.init.data
----------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .init.data

.init.rodata
------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .init.rodata

Initcall levels
---------------
.. kernel-doc:: include/linux/init.h
   :doc: Initcall levels

.initcall
-----------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .initcall

__define_initcall
-----------------
.. kernel-doc:: include/linux/init.h
   :functions: __define_initcall

Linux .exit\* sections
======================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux exit sections

.exit.text
----------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .exit.text

.exit.data
----------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .exit.data

.exitcall.exit
--------------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .exitcall.exit

Linux .ref\* sections
=====================

.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux references to init sections

.ref.text
---------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .ref.text

.ref.data
---------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .ref.data

.ref.rodata
-----------
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: .ref.rodata

Linux section ordering
======================
.. kernel-doc:: include/asm-generic/section-core.h
   :doc: Linux section ordering

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
