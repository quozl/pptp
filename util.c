/* util.c ....... error message utilities.
 *                C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: util.c,v 1.2 2000/12/23 08:32:15 scott Exp $
 */

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include "util.h"

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "(unknown)"
#endif

static void open_log(void) __attribute__ ((constructor));
static void close_log(void) __attribute__ ((destructor));

static void open_log(void) {
  openlog(PROGRAM_NAME, LOG_PID, LOG_DAEMON);
}
static void close_log(void) {
  closelog();
}

#define MAKE_STRING(label) 				\
va_list ap;						\
char buf[256], string[256];				\
va_start(ap, format);					\
vsnprintf(buf, sizeof(buf), format, ap);		\
snprintf(string, sizeof(string), "%s[%s:%s:%d]: %s",	\
	 label, func, file, line, buf);			\
va_end(ap)

void _log(char *func, char *file, int line, char *format, ...) {
  MAKE_STRING("log");
  syslog(LOG_NOTICE, "%s", string);
}

void _warn(char *func, char *file, int line, char *format, ...) {
  MAKE_STRING("warn");
  fprintf(stderr, "%s\n", string);
}

void _fatal(char *func, char *file, int line, char *format, ...) {
  MAKE_STRING("fatal");
  fprintf(stderr, "%s\n", string);
  exit(1);
}
