/* 
   File parser.c contains some functions to parse options into command line.  
 
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
 #include "procps_ptree.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdarg.h>
 #include <time.h>

 #include <sys/types.h>
 #include <sys/param.h>

 #include <unistd.h>

 typedef struct jump_table_struct {

	const char *name; /* a name of long option of command-line */	
        const void *jump; /* a pointer on constant of jump-label */
	
 }jump_table_struct_t;

static int compare_jump_table_structs(const void *ch_p, const void *array_p){

   const jump_table_struct_t*  a = (const jump_table_struct_t*)ch_p;
   const jump_table_struct_t*  b = (const jump_table_struct_t*)array_p;

  return strcmp ( a->name, b->name );
}

static int globalvar_by_get_option_name ( const char buf[],
					        	int *glvar_help,
			  				int *glvar_version,
						        int *glvar_loose_task,
			  				int *glvar_csv_format,
			  				int *glvar_only_pids,
			  				int *glvar_dont_resolve_name,
			  				int *glvar_with_pid )
{
 jump_table_struct_t      findme = { buf, NULL };
 jump_table_struct_t	 *found; /* a jump found labels */
 static const jump_table_struct_t jump_table[] = { /* the table of jump labels */
	{ "key04",     &&case_longHelp },    /* long option --help */
	{ "key07",     &&case_longVersion },  /* long option --version */
	{ "key10",     &&case_longLooseTask },  /* long option --loose-task */
	{ "key13",     &&case_longShowWithPid  }, /* long option --show-with-pid */
	{ "key14",     &&case_longShowOnlyPids }, /* long option --show-only-pids*/
	{ "key15",     &&case_longCsvFormat }, /* long option --csv-format-file  */
	{ "key16",     &&case_longNotResolvedName }, /* long option --not-resolve-name */
        { "key56",     &&case_longVersion }, /* short option -V same as long option --version */
        { "key63",     &&case_longCsvFormat }, /* short option -c same as long option --csv-format-file */
        { "key68",     &&case_longHelp }, /* short option -h same as long option --help */
        { "key6C",     &&case_longLooseTask }, /* short option -l same as long option --loose-task */
        { "key6E",     &&case_longNotResolvedName }, /* short option -n same as long option --not-resolve-name */
        { "key70",     &&case_longShowWithPid }, /* short option -p same as long option --show-with-pid */
        { "key73",     &&case_longShowOnlyPids } /* short option -p same as long option --show-only-pids */
 };
 const int jump_table_count =  /* calculating a number of jump labels into the table   */
			   sizeof(jump_table)/sizeof(jump_table_struct_t);
#ifdef __DEBUG__
 printf("%s: find \'%s\' in jump_table_count=[%d]\n", __FUNCTION__, 
						      findme.name , 
						      jump_table_count );   
#endif /*__DEBUG__*/
 
/* search jump label*/
 found = bsearch ( &findme, jump_table, jump_table_count,
      		      sizeof(jump_table_struct_t), compare_jump_table_structs );

 if(!found) {
    perror("bsearch");	
    return -1; 
 } 	

 goto *(found->jump);

 case_longHelp:
 	if(glvar_help) *glvar_help = 1;
	goto case_Success;

 case_longVersion:
 	if(glvar_help) *glvar_version = 1;
	goto case_Success;

 case_longCsvFormat:
	if(glvar_csv_format) *glvar_csv_format = 1; 
	goto case_Success;
 case_longLooseTask:
	if(glvar_loose_task) *glvar_loose_task = 1;
	goto case_Success;
 case_longShowOnlyPids:
	if(glvar_only_pids) *glvar_only_pids = 1;
	goto case_Success;
 case_longNotResolvedName:
        if(glvar_dont_resolve_name) *glvar_dont_resolve_name = 1;
	goto case_Success;
  case_longShowWithPid:
	if(glvar_with_pid) *glvar_with_pid          = 1;
	goto case_Success;
 case_Success:
	return 0;
}


#define OPTION_NOTFOUND()\
 printf ( "%s: invalid option -- \'%s\'\n", *argv, *p_argv_go )

#define ARGUMENT_NOTFOUND(NAME)\
 printf ( "%s: \'%s\' option required argument\n", *argv, NAME )


/*
   cmdline_opts_parser() parses  command-line options  that were  passed    into 
			 2nd argument and initializes global variables that were
			 passed since 3rd argument.
 	
		     	 On success, this function  returns  a number of options 
		         were recognized. Otherwise, it returns negative value.
*/
int cmdline_opts_parser ( const int argc, char * const *argv, 
			  int *to_show_help,
			  int *to_show_version,
			  int *to_use_loose_task,
			  int *to_use_csv_format,
			  int *to_show_only_pids,
			  int *dont_resolve_name,
			  int *to_show_with_pid ) 
 {
    va_list  ap;
    int      n_parsed_opts  	   = 0;      
    int      to_go          	   = argc-1;   
    char     * const *p_argv_go    = argv+1; 
    char     is_wait_arg    	   = 0;
    int      to_show_with_pid_old  = -1;

    if ( to_show_help )      *to_show_help      = 0;
    if ( to_show_version )   *to_show_version   = 0;	
    if ( to_use_loose_task ) *to_use_loose_task = 0;
    if ( to_use_csv_format ) *to_use_csv_format = 0;
    if ( to_show_only_pids ) *to_show_only_pids = 0;
    if ( dont_resolve_name ) *dont_resolve_name = 0;
    if ( to_show_with_pid )  *to_show_with_pid  = to_show_with_pid_old;

    for ( ; to_go && *p_argv_go; p_argv_go++, to_go-- ) {
        char         optkey[16] = { 0 };         
        char 		*ptr    = *p_argv_go;
	while ( *ptr++ ) 
	      ;
        if ( ( ptr - *p_argv_go ) <= 3 ) {
            const char *ptr = *p_argv_go;
            snprintf( optkey, 16, "key%02X", *(ptr+1) );       
        }
	if ( ( ptr - *p_argv_go ) > 3 )	{	
	   snprintf( optkey, 16, "key%02d", (ptr-*p_argv_go)-3 );
        }
	if( globalvar_by_get_option_name ( optkey, 
						 to_show_help,
					         to_show_version,
					  	 to_use_loose_task,
			  			 to_use_csv_format,
			  			 to_show_only_pids,
			  			 dont_resolve_name,
			  			 to_show_with_pid  ) < 0 ) {
	     OPTION_NOTFOUND();
	     n_parsed_opts = -1;
	     goto out;
	 }
         if ( *to_show_with_pid != to_show_with_pid_old ) {
           int boolean_value = (int)(*(++p_argv_go) == NULL);
           const char *ptr = *p_argv_go;
           for ( ; !boolean_value && ptr && *ptr ; ptr++ ){
             if(isdigit( *ptr )) continue;
             boolean_value++;
           }
           if ( boolean_value ) {
		ARGUMENT_NOTFOUND("-p,--show_with_pid");
                *to_show_with_pid = 0;
	        n_parsed_opts = -1;
	        goto out;
           }
	   *to_show_with_pid = atoi(*p_argv_go);		
	   to_show_with_pid_old = *to_show_with_pid;	
         }
         n_parsed_opts++;
    }	   
out:
    return  EXIT_SUCCESS;
 }

/*eof*/

