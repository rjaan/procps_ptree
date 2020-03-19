 /* 
   the file output.c contains functions that output information about executed 
		     processes and threads. They are using standard I/O streams 
		     such as stdout, stderr. 

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

#include <proc/readproc.h>
#include <proc/escape.h>
#include <proc/sysinfo.h>


PRINT_INFO(pid);
PRINT_INFO(pid);
PRINT_INFO(ppid);		
PRINT_INFO(stat);
PRINT_INFO(user);
PRINT_INFO(pcpu);
PRINT_INFO(pmem);
PRINT_INFO(stime);			
PRINT_INFO(time);
PRINT_INFO(comm);
PRINT_INFO(thread);

typedef void ( * pr_handler_t ) ( char *restrict const outbuf, 
			       		      const proc_t *restrict const pp );

typedef struct pr_list {
   pr_handler_t	 pr;
   const char 	*fmt;
   const char 	*header;	
   char		 sep_ch[2];
}pr_list_t; 


static const pr_list_t pr_list_proc[]= {
{ pr_pid,  "%6s",  "PID",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },
{ pr_ppid, "%6s",  "PPID",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },		
{ pr_stat, "%7s",  "STAT",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },
{ pr_user, "%11s", "USER",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },
{ pr_pcpu, "%4s",  "%%CPU",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },
{ pr_pmem, "%4s",  "%%MEM",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },
{ pr_stime,"%16s", "START_TIME",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },			
{ pr_time, "%12s", "TIME",{ SEPORATOR_SYMBOL_SPACE, SEPORATOR_SYMBOL_SEMI } },
{ NULL, NULL, "",{ 0, 0 } }	  	
};

void show_header ( void )
{
    const pr_list_t *fmt_list = pr_list_proc;

    if ( glvar_to_show_only_pids ) {
         return;
    }
 
    fputc ( '\n', stdout );
    fflush(stdout);
    while ( fmt_list && fmt_list->pr ) {
          fprintf ( stdout, fmt_list->fmt, fmt_list->header );
          fputc ( fmt_list->sep_ch[(glvar_to_use_csv_format ? 1 : 0)], stdout );		
	  fmt_list++;
    }
    fprintf ( stdout, " COMMAND");
    fflush(stdout);	
    fputc ( '\n', stdout );
    fflush(stdout);
}

void show_one_proc( const proc_t *p, int level, int is_show )
{
    int i = level;
    const pr_list_t *fmt_list = pr_list_proc;  
    static char outbuf[PATH_MAX] = { 0 };

    if(!is_show) return;
	
    while ( fmt_list && fmt_list->pr ) {
    	bzero( outbuf, PATH_MAX );
	fmt_list->pr( outbuf, p );  
	fprintf ( stdout, fmt_list->fmt, outbuf );
	if ( glvar_to_show_only_pids ) {
	  fputc ( '\n', stdout ); 
	  fflush(stdout);
	  return; 
        }
	fputc ( fmt_list->sep_ch[(glvar_to_use_csv_format ? 1 : 0)], stdout );		
	fmt_list++;
    }

    if ( !glvar_to_use_csv_format )    
       while ( i-- ) {
	  int space = 3;
	  while ( space-- )
	      fputc ( ' ', stdout );
       }

    pr_comm ( outbuf, p );  	
    fprintf ( stdout, "%s\n", outbuf );
    fflush(stdout);	

}

#define ROWLENGTH  240 /* row length */

/******************************************************************************
 * Output formated functins						      *	
 ******************************************************************************/
 static void pr_pid ( char *restrict const outbuf, const proc_t *restrict const pp ) {
   if ( pp->tid != pp->tgid ) {
      pr_thread ( outbuf, pp );	
      return;
   }
   (void)snprintf(outbuf, ROWLENGTH, "%6u", pp->tgid);
 }

 static void pr_ppid(char *restrict const outbuf, const proc_t *restrict const pp) {
  if ( pp->tid != pp->tgid ) { 
     (void)snprintf(outbuf, ROWLENGTH, "%6u", pp->tgid);
     return;	  
  } 	
  (void)snprintf(outbuf, ROWLENGTH, "%6u", pp->ppid);
 }

 static void pr_comm(char *restrict const outbuf, const proc_t *restrict const pp) {
   int rightward = 16;	
   (void)escape_command ( outbuf, pp, ROWLENGTH, &rightward, ESC_DEFUNCT );
 }


 static void pr_user(char *restrict const outbuf, const proc_t *restrict const pp){
    int rightward = 10;

    if ( !glvar_dont_resolve_name )  
       (void)escape_str(outbuf, pp->euser, ROWLENGTH, &rightward);
    else
       (void)snprintf ( outbuf, ROWLENGTH, "%6u", pp->euid );
 }

 static void pr_pcpu(char *restrict const outbuf, const proc_t *restrict const pp){
  /* seconds of process life */
   unsigned long long seconds    = glvar_seconds_since_boot - pp->start_time / Hertz; 
  /* jiffies used by this process */
   unsigned long long total_jiffies = pp->utime + pp->stime;
  /* scaled %cpu, 999 means 99.9% */
   unsigned int pcpu;

   if ( !seconds ) { /* to except the division by zero error */
     (void)snprintf(outbuf, ROWLENGTH, "%2u.0", 0 );  	
      return;	
   }	

   pcpu = (total_jiffies * 1000ULL / Hertz) / seconds;

   if (pcpu > 999U ) {
    (void)snprintf(outbuf, ROWLENGTH, "99.9" );
    return; 
   }
   (void)snprintf(outbuf, ROWLENGTH, "%2u.%u", pcpu/10U, pcpu%10U);
 }

 static void pr_pmem(char *restrict const outbuf, const proc_t *restrict const pp){
/* scaled %mem, 999 means 99.9% */
  unsigned long pmem;
  if( kb_main_total ) { /* to except the division by zero error */
   (void)snprintf(outbuf, ROWLENGTH, "DZ.ERR" ); 	
  }	
  pmem = pp->vm_rss * 1000ULL / kb_main_total;	
  if ( pmem > 999) pmem = 999;

  (void)snprintf(outbuf, ROWLENGTH, "%2u.%u", (unsigned)(pmem/10), (unsigned)(pmem%10)); 
}

static void pr_stime(char *restrict const outbuf, const proc_t *restrict const pp){

 struct tm *tm; /* date and time to broken-down time*/
 int our_yday;  /* our year day */
 int our_year;   /* our year */

 const char *fmt = NULL; /* Объявить формат вывода для strftime() */

/* seconds since 1970 of boot system */
 time_t  seconds_since_1970_of_boot = glvar_seconds_since_1970 - 
						       glvar_seconds_since_boot;
/* seconds since 1970 of start process */
 time_t  seconds_since_1970_of_life = seconds_since_1970_of_boot + 
						       (pp->start_time / Hertz);

 tm = localtime(&glvar_seconds_since_1970);
 our_yday = tm->tm_yday;
 our_year = tm->tm_year;

 tm = localtime ( &seconds_since_1970_of_life ); 
 if( our_yday != tm->tm_yday ) fmt = "%m-%d %H:%M:%S";  /* mm-dd HH:MM:SS */
 if( our_year != tm->tm_year ) fmt = "%Y-%m-%d %H:%M:%S";  /* YY-mm-dd HH:MM:SS */
 if( fmt == NULL ) fmt = "%H:%M:%S"; /* HH:MM:SS*/

 (void)strftime(outbuf, 42, fmt, tm  ); 
}

static void pr_time ( char *restrict const outbuf, const proc_t *restrict const pp ) {

    unsigned int  dd_chars, dd, mm, hh, ss;	
    unsigned long cpu_seconds;

    if ( !Hertz ) {
      snprintf(outbuf, ROWLENGTH, "DIV:ZERO:ERR" );	
      return; 
    }
	
    cpu_seconds = (pp->utime + pp->stime) / Hertz;

    ss = cpu_seconds % 60;

    cpu_seconds /= 60;	   
    mm = cpu_seconds % 60;

    cpu_seconds /= 60;
    hh = cpu_seconds % 24; 

    dd = cpu_seconds / 24;
    dd_chars  = ( dd ? snprintf(outbuf, 240, "%u-", dd) : 0 );

    (void)snprintf ( outbuf+dd_chars, ROWLENGTH, "%02u:%02u:%02u", hh, mm, ss );
}

static void pr_stat(char *restrict const outbuf, const proc_t *restrict const pp ) {

    int offset = 0;

    *(outbuf+offset++) = pp->state; /* set one of state-codes:
					 D    Uninterruptible sleep (usually IO)
 					 R    Running or runnable (on run queue)
 					 S    Interruptible sleep (waiting for an event to complete)
 					 T    Stopped, either by a job control signal or because it is being traced.
 					 X    dead (should never be seen)
 					 Z    Defunct ("zombie") process, terminated but not reaped by its parent */
/* add state-code to state-codes set */
    if(pp->nice < 0)                  *(outbuf+offset++) = '<'; /* high-priority (not nice to other users) */
    if(pp->nice > 0)                  *(outbuf+offset++) = 'N'; /* low-priority (nice to other users) */
    if(pp->vm_lock)                   *(outbuf+offset++) = 'L'; /* pages locked into memory (for real-time and custom IO) */
    if(pp->session == pp->tgid)       *(outbuf+offset++) = 's'; /*  session leader */
    if(pp->nlwp > 1)                  *(outbuf+offset++) = 'l'; /*  multi-threaded */
    if(pp->pgrp == pp->tpgid)         *(outbuf+offset++) = '+'; /*  in foreground process group */

    *(outbuf+offset) = '\0'; /* end of line */
}

static void pr_thread ( char *restrict const outbuf, const proc_t *restrict const pp ) {
  (void)snprintf(outbuf, ROWLENGTH, "%6u", pp->tid);
}

/*eof*/

