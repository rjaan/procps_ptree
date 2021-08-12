/* 
   The file main.c contains a program shows hierarchy of processes along with 
                   their threads and is modified source code of procps-ng and 
	           demonstrates some facilities of library procps.  
 
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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

 */

#include "procps_ptree.h"


#include <proc/readproc.h>
#include <proc/escape.h>
#include <proc/sysinfo.h>

static int to_show_help;
static int to_show_version;
static int to_use_loose_task;
static int to_show_with_pid;

int glvar_to_use_csv_format;
int glvar_to_show_only_pids;
int glvar_dont_resolve_name;

#define PRINT_VARIABLE(VARIABLE)\
	printf( "%s(): %s = %d\n" , __FUNCTION__, # VARIABLE, VARIABLE )

static void help_message ( void );
static void  global_vars_init ( void );

static  proc_t	 	 **processes;

static proc_data_t* procps_tab_open ( void );
static void procps_tab_close ( proc_data_t  *pd_p );
static int procps_plist_sort ( proc_data_t  *pd_p );

static void show_tree ( const int self, const int n, const int level, 
					  const int have_sibling, int is_fill ); 

int
main ( int argc, char **argv )
{
  int 		   retval  = EXIT_SUCCESS;
  proc_data_t*     pd_p  = NULL;
  int              parent; 

  to_show_help      ^= to_show_help;
  to_show_version   ^= to_show_version;
  to_use_loose_task ^= to_use_loose_task;
  to_show_with_pid  ^= to_show_with_pid;
  
  if ( (retval=cmdline_opts_parser ( argc, argv, 
				  	   &to_show_help,
			          	   &to_show_version,
					   &to_use_loose_task,
					   &glvar_to_use_csv_format,
					   &glvar_to_show_only_pids,
					   &glvar_dont_resolve_name,
					   &to_show_with_pid   )) != EXIT_SUCCESS ) {
     help_message();
     exit(EXIT_FAILURE);
  }
 
  if ( to_show_help ) {
       help_message();
       exit(0);
  }

  if ( to_show_version ) {
       VERSION_BANER();
       exit(0);
  }

  if( !glvar_to_show_only_pids )
  		VERSION_SHORTBANER();

  global_vars_init ();
 
  pd_p = procps_tab_open ();
  if ( pd_p == NULL ) {
     fprintf(stderr,"%s: can\'t open procps!\n", *argv );
     return EXIT_FAILURE;
  }

  if ( procps_plist_sort ( pd_p ) ) {
        fprintf(stderr, "can\'t sort a proclist\n");
        retval = EXIT_FAILURE;	 
        goto close_proctab; 	
  }

#ifdef __DEBUG__
  PRINT_VARIABLE(to_show_version);
  PRINT_VARIABLE(to_use_loose_task);
  PRINT_VARIABLE(glvar_to_use_csv_format);
  PRINT_VARIABLE(glvar_to_show_only_pids);
  PRINT_VARIABLE(glvar_dont_resolve_name);
  PRINT_VARIABLE(to_show_with_pid);
#endif /*__DEBUG__*/

  show_header();

  parent = pd_p->n;
  while ( parent-- ) {
     int subparent = pd_p->n;
     while ( subparent-- ) {
         if ( processes[subparent]->XXXID == processes[parent]->ppid ) {
	    goto not_root;
         }
     }
     show_tree ( parent, pd_p->n, 0, 0, 0 ); 
not_root:
	 ;
  }
     
close_proctab:
  
  procps_tab_close ( pd_p );

  if ( !glvar_to_show_only_pids )
     if( retval == EXIT_FAILURE ) puts ("Failure");
     else puts ("Success");

  return retval;
}

/*
[--help] [--loose-task][--csv-format][--show-only-pid][--not-resolve-name][--show-with-pid root_pid or -p root_pid]
*/

static void help_message ( void )
{
   printf ("%s - \
shows hierarchy of process along with  their tasks\n", PACKAGE_STRING );
  printf ("Usage: %s [OPTION]\n", PACKAGE_STRING );
  printf ("\
Options:\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit\n\
  -l, --loose-task           show threads as if they were processes\n\
  -c, --csv-format-file      output processes and threads in the CSV format\n\
  -s, --show-only-pids       only output pids of processes and threads\n\
  -n, --not-resolve-name     don\'t resolve a user name at user id.\n\
  -p, --show-with-pid        shows a tree of processes and threads from this pid\n\
");

}

/* some defines and user types to be required by sorting proctab */
typedef int (*sr_t)(const proc_t* P, const proc_t* Q);

typedef struct sort_node {

  struct sort_node    *next;
  int (*sr)(const proc_t* P, const proc_t* Q); /* sort function */

} sort_node_t;

#define CMP_SMALL(NAME) \
static int sr_ ## NAME (const proc_t* P, const proc_t* Q) { \
    return (int)(P->NAME) - (int)(Q->NAME); \
}

#define CMP_INT(NAME) \
static int sr_ ## NAME (const proc_t* P, const proc_t* Q) { \
    if (P->NAME < Q->NAME) return -1; \
    if (P->NAME > Q->NAME) return  1; \
    return 0; \
}

static  int  		flag;
static  PROCTAB 	*restrict proctab_p;

unsigned long   	glvar_seconds_since_boot;
time_t			glvar_seconds_since_1970;

static  sort_node_t  	  *sort_list;

static void  global_vars_init ( void )
{

  flag = ( PROC_FILLMEM | PROC_FILLSTATUS | PROC_FILLSTAT | 
	   PROC_FILLCOM | PROC_FILLUSR );

  if ( to_use_loose_task ) {
     flag |= PROC_LOOSE_TASKS;
  }
   
  if ( glvar_dont_resolve_name ) {
     flag &= ~PROC_FILLUSR; 
  }

  glvar_seconds_since_boot    = uptime(0,0);
  glvar_seconds_since_1970    = time(NULL);

  proctab_p		      = NULL;
  sort_list                   = NULL;
   
  meminfo ();
} 

static int want_this_proc_nop_cb ( proc_t *dummy ) {
  (void*)dummy;
  return 1;
}

static int want_this_task_nop_cb ( proc_t *dummy ) {
  (void*)dummy;
  return 1;

} 

static proc_data_t* procps_tab_open ( void )
{
  proc_data_t	 *pd_p;

  proctab_p = openproc ( flag );   
  if ( !proctab_p ) {
    fprintf(stderr, "failure: can not access /proc.\n");
    return NULL;
  }

  if( flag & PROC_LOOSE_TASKS ) {
    pd_p = readproctab2 ( want_this_proc_nop_cb, 
			  want_this_task_nop_cb,  proctab_p ); 	
  }else{
    pd_p = readproctab2 ( want_this_proc_nop_cb, 
			  		   NULL,  proctab_p ); 		
  }

  processes = pd_p->tab;

  return pd_p;
}

static void procps_tab_close ( proc_data_t	 *pd_p )
{   
   unsigned long      address_min = 0UL;
   if ( pd_p ) {
     unsigned int       ntab        = pd_p->n;
     proc_t             **tab_pp    = pd_p->tab ? pd_p->tab : (proc_t **)NULL;
     proc_t             *data_p     = (proc_t *)NULL;
     while ( tab_pp && ntab-- ) {
       proc_t   *p = tab_pp[ntab];   
       if ( !address_min ) {
	   address_min = (unsigned long)p;
       }
       if( (unsigned long)p < address_min ) {
	    address_min = (unsigned long)p;  
	    data_p = p;	
       }

       if( !(flag & PROC_LOOSE_TASKS) && p->cmdline )
		free((void*)*p->cmdline);
       if (p->environ)
		free((void*)*p->environ);
     }
     if ( data_p ) {
      pd_p->n = 0;
      pd_p->ntask = 0;
      pd_p->nproc = 0;
     }
     closeproc( proctab_p ); 
   }
}

CMP_SMALL(ppid);
CMP_INT(start_time);

static int compare_two_procs(const void *a, const void *b ) {

   sort_node_t *tmp_list = sort_list;

   while ( tmp_list != (sort_node_t *)NULL ) {
     int result = (*tmp_list->sr)(*(const proc_t *const*)a, *(const proc_t *const*)b);
     if( result ) return result;	
     tmp_list = tmp_list->next;
   }
   return 0; /* no conclusion */
}


static int procps_plist_sort ( proc_data_t 	*pd_p )
{
  sort_node_t  *tmp_list = sort_list;	

  tmp_list = malloc( sizeof(sort_node_t));  /* ppid */
  if ( !tmp_list ) {	
    return -1;	
  }	
  tmp_list->sr   = sr_ppid;
  tmp_list->next = sort_list; /*  0xdeadbeef */
  sort_list      = tmp_list; 
  
  tmp_list = malloc ( sizeof(sort_node_t) );  /* start_time */
  if ( !tmp_list ) {	
    free( tmp_list );	
    return -1;	
  }	 
  tmp_list->sr   = sr_start_time;
  tmp_list->next = sort_list;
  sort_list      = tmp_list; 
  
  qsort ( pd_p->tab, pd_p->n, sizeof(proc_t*), compare_two_procs );

  if ( sort_list->next ) { 
     free ( sort_list->next ); 
  }
  free ( sort_list );
  
  return 0;
}

/**
  * recurse-show procs tree
  * 
  **/
static void show_tree ( const int self, const int n, const int level, const int have_sibling, int is_fill ) 
{
    int 	i=0;
    int 	to_fill = is_fill;
    
    if( !to_fill ) {
      if ( to_show_with_pid == -1 ) to_fill = 1;
      else
      if ( to_show_with_pid == processes[self]->tgid ) to_fill = 1;
    }

    show_one_proc( processes[self], level, to_fill );

    if ( !to_use_loose_task ) {
      proc_t 	*task = NULL ; 
      while ( readtask ( proctab_p, processes[self], task ) ) 
        {
	     if ( task )
	      {
	        if( task->tid != task->tgid ) { /* this isn't my process */
		  show_one_proc( task, level+1, to_fill );
		}
		free(task);
	        task=NULL; 
	      }
        } 
      
    }
    i=0;	
    for (; i < n; i++ ) {
	int have_brother = 0;
        if( processes[self]->XXXID != processes[i]->ppid ) continue;
	if( (i+1) < n ) {
   	  if(processes[i+1]->ppid == processes[i]->ppid ) have_brother++;
	}
	show_tree ( i, n, level+1, have_brother, to_fill );  
    }  
	
}


/*eof*/

