/* 
   The file procps_ptree.h contains some function prototypes and macros .  
 
   Copyright (C) 2012  Andrey Y. Rzhavskov  <mailto:rjaan@yandex.ru>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/
#ifndef __PROCPS_PTREE_H
#define __PROCPS_PTREE_H	1

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include <sys/types.h>
#include <sys/param.h>

#include <unistd.h>

extern int cmdline_opts_parser ( const int argc, char * const *argv, 
			  int *to_show_help,
			  int *to_show_version,
			  int *to_use_loose_task,
			  int *to_use_csv_format,
			  int *to_show_only_pids,
			  int *dont_resolve_name,
			  int *to_show_with_pid ); /* parses cmd-line 
								options and 
						    		initialize lobal vars */


#define HAVE_INVALID_OPTION  "invalid option"


#define VERSION_BANER()\
puts ("The " PACKAGE " version " VERSION ".\n"\
      "To report a bug please contact " PACKAGE_BUGREPORT ".");

#define VERSION_SHORTBANER()\
puts ("The " PACKAGE " version " VERSION ".");

extern unsigned long   	glvar_seconds_since_boot;
extern time_t		glvar_seconds_since_1970;
extern int 		glvar_to_use_csv_format;
extern int 		glvar_to_show_only_pids;
extern int		glvar_dont_resolve_name;


#define PRINT_INFO(NAME) \
static void pr_ ## NAME (char *restrict const outbuf,\
					const proc_t *restrict const pp);

#define SEPORATOR_SYMBOL_SPACE 		' '
#define SEPORATOR_SYMBOL_SEMI  		';'
#define SEPORATOR_SYMBOL_RETURN  	'\n'

#define SEPORATOR_DEFAULT		NULL

#include <proc/readproc.h>

extern void show_header ( void );
extern void show_one_proc ( const proc_t *p, int level, int is_show );

#endif /*__PROCPS_PTREE_H*/

