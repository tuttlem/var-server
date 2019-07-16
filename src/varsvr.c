#include <unistd.h>

#include "./log.h"
#include "./daemon.h"

/** Program entry point */
int main(int argc, char *argv[]) {

  log_init();
 
  if (daemon_init() != ERR_SUCCESS) {
    _exit(1);
  }

  daemon_run();
  daemon_teardown();
  log_teardown();
 
  _exit(0);
  return 0;
}
