##  configure.as is an input file and is a shell script with macro instructions.
##         
##  Copyright (C) 2012  Andrey Y. Rzhavskov  <mailto:rjaan@yandex.ru>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

AC_INIT([procps_ptree], [1.4], [rjaan@yandex.ru])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.
AC_PROG_CC
AC_LANG_C
AC_PROG_AWK
##AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
#LIBS="-L /usr/lib/x86_64-linux-gnu/ -lprocps"

AC_CHECK_LIB(procps,openproc)
AC_CHECK_LIB(procps,readproctab2)
AC_CHECK_LIB(procps,readtask)
AC_CHECK_LIB(procps,closeproc)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([proc/readproc.h])
AC_CHECK_HEADERS([proc/escape.h])
AC_CHECK_HEADERS([proc/sysinfo.h])

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 man/Makefile
])
AC_OUTPUT
