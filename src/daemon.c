
#include "./daemon.h"

pid_t vs_daemon_pid;
int vs_daemon_running;

/**
 * The var server's signal handler 
 */
void daemon_signal_handler(int sig) {
  log_info("Server received signal %d", sig);

  switch (sig) {
    case SIGHUP:
    case SIGTERM:
      vs_daemon_running = 0;
      break;
  }
}


/**
 * Daemonizes this application so that it will run in the background
 */
int daemon_init() {

  vs_daemon_running = 0;

  /* first fork */
  pid_t pid = fork();

  if (pid == -1) {
    log_error("Unable to fork (errno=%d)", errno);
    return ERR_DMINIT;
  } else if (pid != 0) {
    _exit(0);
  }

  /* start a new session */
  if (setsid() == -1) {
    log_error("Unable to become session leader (errno=%d)", errno);
    return ERR_DMINIT;
  }

  /* fork again */
  signal(SIGHUP, SIG_IGN);
  pid = fork();

  if (pid == -1) {
    log_error("Unable to secondary fork (errno=%d)", errno);
    return ERR_DMINIT;
  } else if (pid == 0) {
    _exit(0);
  }

  vs_daemon_pid = pid;

  /* change to a known location */
  if (chdir("/") == -1) {
    log_error("Unable to change working directory (errno=%d)", errno);
    return ERR_DMINIT;
  }

  /* attach the signal handlers */
  signal(SIGCHLD, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGHUP, daemon_signal_handler);
  signal(SIGTERM, daemon_signal_handler);
  
  /* set the umask */
  umask(0);

  /* handle the standard file descriptors */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  if (open("/dev/null", O_RDONLY) == -1) {
    log_error("Unable to re-open stdin (errno=%d)", errno);
    return ERR_DMINIT;
  }

  if (open("/dev/null", O_WRONLY) == -1) {
    log_error("Unable to re-open stdout (errno=%d)", errno);
    return ERR_DMINIT;
  }

  if (open("/dev/null", O_RDWR) == -1) {
    log_error("Unable to re-open stderr (errno=%d)", errno);
    return ERR_DMINIT;
  }

  vs_daemon_running = 1;

  return ERR_SUCCESS;
}

/**
 * Tears down the daemon process
 */
int daemon_teardown() {
  log_info("Killing server pid %d", vs_daemon_pid);
  kill(vs_daemon_pid, SIGTERM);

  return ERR_SUCCESS;
}

/**
 * Runs the daemon process
 */
int daemon_run() {
  log_info("Server process is running");

  while (vs_daemon_running) {
    sleep(5);
  }

  log_info("Server process is stopping");

  return ERR_SUCCESS;
}
