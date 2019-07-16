#ifndef __varsvr_daemon_h_

#define __varsvr_daemon_h_

#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "./log.h"
#include "./errors.h"

/**
 * Daemonizes this application so that it will run in the background
 */
int daemon_init();

/**
 * Tears down the daemon process
 */
int daemon_teardown();

/**
 * Runs the daemon process
 */
int daemon_run();

#endif /* __varsvr_daemon_h_ */
