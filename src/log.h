#ifndef __varsrv_log_h_

#define __varsrv_log_h_

#include <stdarg.h>
#include <syslog.h>

#define log_debug(...) _log_msg(LOG_DEBUG, __VA_ARGS__)
#define log_info(...) _log_msg(LOG_INFO, __VA_ARGS__)
#define log_warn(...) _log_msg(LOG_WARNING, __VA_ARGS__)
#define log_error(...) _log_msg(LOG_PERROR, __VA_ARGS__)

/**
 * Starts the logging system up
 */
void log_init(void);

/**
 * Closes down the logging system
 */
void log_teardown(void);

/**
 * Message logging implementation
 */
void _log_msg(int priority, const char *format, ...);

#endif /* __libced_log_h_ */
