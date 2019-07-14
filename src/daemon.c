#include "../include/log.h"
#include "../include/typesys.h"

/** Program entry point */
int main(int argc, char *argv[]) {

  log_init();

  vsval *s = NULL;

  if (vsval_create("text", &s) != ERR_SUCCESS) {
    log_error("Failed to create string");
    exit(2);
  }

  if (vsval_set_text(s, "Hello, world!") != ERR_SUCCESS) {
    log_error("Failed to set value");
    exit(3);
  }

  vsval_print(s);
  
  vsval_destroy(&s);

  log_debug("Value destroyed");

  log_teardown();
   return 0;
}
