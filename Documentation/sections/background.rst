======================
ELF section background
======================

About
=====

The purpose of this chapter is to help those not familiar with ELF to brush up
the latest ELF specifications in order to help understand how Linux uses and
defines its own ELF sections.

Standardized ELF
================

The first publication documenting ELF was UNIX System Laboratories' (USL)
*System V Release 4 Application Binary Interface* (`SRV4 ABI`_) specification.
Originally ELF was only a small part of the SRV4 ABI, with time however new
specifications only put focus on ELF, such was the case of the *TIS Portable
Formats Specification version 1.2* (`TIS 1.2`_). As of TIS 1.2, ELF was
supplemented with processor specific ELF addendums, available on the *Linux
Foundation referenced specification page* (`LF ref page`_). The latest ELF
specification is the *System V Application Binary Interface - DRAFT - 24 April
2001* (`gabi4`_).

.. _SRV4 ABI: http://www.sco.com/developers/devspecs/gabi41.pdf
.. _TIS 1.2: https://refspecs.linuxbase.org/elf/elf.pdf
.. _LF ref page: https://refspecs.linuxbase.org/
.. _gabi4: https://refspecs.linuxbase.org/elf/gabi4+/contents.html

ELF views on Linux
==================

There are two views which can be used for inspecting data in an ELF file, a
Linking view, and an Execution view. A Section Header Table enables one to
describe an object using the Linking view while a Program Header Table enables
one to describe an object using the Execution view. The views are not mutually
exclusive. For instance, vmlinux can be viewed under both views, ``readelf -S
vmlinux`` for the Linking view, and ``readelf -l vmlinux`` for the Execution
view.  In Linux only the vmlinux file will have an Execution view, even modules
lack an Execution view given that vmlinux is the only file that describes how
the the kernel runs from the start.  All other Linux kernel object files have
an available Linking view.

Under the Linking view, the Section Header Table describes all available
sections. The Section Header Table is an array of ELF section header data
structures. If on a 32-bit system this is ``struct elf32_shd``, if on a 64-bit
this is ``struct elf64_shdr``. Sections are only visible on object files that
have a Linking view, since all Linux kernel files have Linking view, all kernel
objects have ELF sections.

Limitations on ELF sections
===========================

We provide a summary on the limitations of ELF sections. Refer to the public
ELF specifications for details. Note that 64-bit limitations may depend
on processor specific section attributes to be used, refer to your processor
specification if unsure.

Its worth elaborating on the limitations on the name of an ELF section:
ELF section names are stored as strings as per the ELF specification, and
as can be expected, these don't have explicit limitations. The implicit
limitation then depends on the size of an ELF object file and ELF section.

If using really large kernels or objects with large amounts of sections one
would still need to be sure that ELF loader in charge of loading the Linux
kernel is properly updated to handle coping with the latest ELF extensions.

   .. flat-table:: Limitations on ELF Sections

      * - Section attribute
        - 32-bit
        - 64-bit

      * - ELF section name
        - Size of an ELF section
        - Size of an ELF section

      * - Size of an ELF section
        - 4 GiB
        - 16 EiB

      * - Max number of sections in an object file
        - 4 GiEntries (4294967296)
        - 16 EiEntries (18446744073709551616)

Program specific ELF sections
=============================

The ELF specification allows for a section type to be specified as
*Program specific section*, defined as ``SHT_PROGBITS``. This sections type
enables programs to customize sections for their own use. In assembly this
specified ``@progbits`` on most architectures, on ARM this is ``%progbits``.

``SHT_PROGBITS`` is used by Linux for defining and using Linux ELF sections.

Special ELF Sections
====================

The ELF specification defines *Special ELF Sections* on chapter 4 (`gabi4
ch4`_). These are defined as sections which hold program and control
information. Of these sections, a few have the section type as
``SHT_PROGBITS``. This enables Linux to *further customize* use of the section
beyond what the ELF specification suggests.

.. _gabi4 ch4: https://refspecs.linuxbase.org/elf/gabi4+/ch4.sheader.html#special_sections
