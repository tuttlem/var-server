#include "./log.h"
#include "./bintree.h"
#include "./typesys.h"

/** Program entry point */
int main(int argc, char *argv[]) {

  log_init();

  vsval *s = NULL;
  vsval *r = NULL;
  bintree *b = bintree_create();


  if (vsval_create("text", &s) != ERR_SUCCESS) {
    log_error("Failed to create string");
    exit(2);
  }


  if (vsval_set_text(s, "Hello, world!") != ERR_SUCCESS) {
    log_error("Failed to set value");
    exit(3);
  }

  bintree_insert(b, "s", s);
  r = bintree_find(b, "s");
  
  vsval_print(r);
  
  vsval_destroy(&s);
  bintree_destroy(&b);

  log_debug("Value destroyed");

  log_teardown();
   return 0;
}
