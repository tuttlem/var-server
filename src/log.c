#include "./log.h"

const char *log_identity = "varsrv";


/**
 * Starts the logging system up
 */
void log_init(void) {
  openlog(log_identity, LOG_NOWAIT, LOG_USER);
}

/**
 * Closes down the logging system
 */
void log_teardown(void) {
  closelog();
}

/**
 * Message logging implementation
 */
void _log_msg(int priority, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsyslog(priority, format, ap);
  va_end(ap);
}

