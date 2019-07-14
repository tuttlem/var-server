#ifndef __varsrv_typesys_h_

#define __varsrv_typesys_h_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./errors.h"

#define VSVAL_DEFAULT_LENGTH 16

typedef struct _tag_type_desc {
  unsigned int id;
  const char *name;
  unsigned int length;

  unsigned int is_numeric;
  unsigned int is_floating;
  unsigned int is_text;
  unsigned int is_binary;
  unsigned int is_var_length;
} type_desc;

typedef struct _tag_vsval {
  unsigned int type_id;
  void *data;
  unsigned int length;
} vsval;

int vsval_create(char *name, vsval **v);

int vsval_destroy(vsval **v);

int vsval_set(vsval *v, unsigned int type_id, void *data, unsigned int length);

int vsval_set_null(vsval *v);
int vsval_set_numeric(vsval *v, int i);
int vsval_set_float(vsval *v, float f);
int vsval_set_double(vsval *v, double f);

int vsval_set_text(vsval *v, const char *s);

int vsval_print(vsval *v);
#endif /*__varsrv_typesys_h_*/
