/* util.h ....... error message utilities.
 *                C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: util.h,v 1.2 2002/10/16 04:45:36 quozl Exp $
 */

#ifndef INC_UTIL_H
#define INC_UTIL_H

/* log_string is an identifier for this pptp process, passed from
   command line using --log-string=X, and included with every log message.
   Useful for people with multiple pptp sessions open at a time */
extern char * log_string;

void _log(char *func, char *file, int line, char *format, ...)
     __attribute__ ((format (printf, 4, 5)));
void _warn(char *func, char *file, int line, char *format, ...)
     __attribute__ ((format (printf, 4, 5)));
void _fatal(char *func, char *file, int line, char *format, ...)
     __attribute__ ((format (printf, 4, 5))) __attribute__ ((noreturn));

#define log(format, args...) \
	_log(__FUNCTION__,__FILE__,__LINE__, format , ## args)
#define warn(format, args...) \
	_warn(__FUNCTION__,__FILE__,__LINE__, format , ## args)
#define fatal(format, args...) \
	_fatal(__FUNCTION__,__FILE__,__LINE__, format , ## args)

#endif /* INC_UTIL_H */
