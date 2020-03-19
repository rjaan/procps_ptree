# Installation Instructions

Copyright (C) 1994, 1995, 1996, 1999, 2000, 2001, 2002, 2004, 2005,
2006, 2007, 2008, 2009 Free Software Foundation, Inc. 

   Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without warranty of any kind.

## Basic Installation

   Briefly, the shell commands *`./configure; make; make install'* should
configure, build, and install this package.  

   The simplest way to compile this package is:

  1. *`cd'* to the directory containing the package's source code and type
     *`./configure'* to configure the package for your system.

     Running *`configure'* might take a while.  While running, it prints
     some messages telling which features it is checking for.

  2. Type *`make'* to compile the package.

  3. Type *`make install'* to install the programs and any data files and
     documentation.  When installing into a prefix owned by root, it is
     recommended that the package be configured and built as a regular
     user, and only the *`make install'* phase executed with root
     privileges.

  4. You can remove the program binaries and object files from the
     source code directory by typing *`make clean'*.  To also remove the
     files that *`configure'* created (so you can compile the package for
     a different kind of computer), type *`make distclean'*.  There is
     also a *`make maintainer-clean'* target, but that is intended mainly
     for the package's developers.  If you use it, you may have to get
     all sorts of other programs in order to regenerate files that came
     with the distribution.

  5. You can also type *`make uninstall'* to remove the installed
     files again.  In practice, not all packages have tested that
     uninstallation works correctly, even though it is required by the
     GNU Coding Standards.

## Compilers and Options
 
   Script *`./configure'* does not use any required options .

