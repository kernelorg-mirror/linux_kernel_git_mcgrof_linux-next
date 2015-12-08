=========================
Linux Kernel ELF sections
=========================

This book documents the different ELF sections used on the Linux kernel.
We start off by providing references to how ELF was standardized, references
to the standards on ELF sections, review limitations of ELF sections, and
finally how Linux uses ELF sections in the Linux kernel. Certain important
Linux ELF sections are documented carefully: we describe the goal of the
ELF section, and address concurrency considerations when applicable. A few
common a set of Linux helpers for ELF sections are also documented.

.. toctree::
   :maxdepth: 4

   background
   section-core
   ranges
   linker-tables
