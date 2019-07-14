#ifndef __varsrv_typesys_h_

#define __varsrv_typesys_h_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./errors.h"

#define VSVAL_DEFAULT_LENGTH 16

#define VST_NULL      0x00000000
#define VST_NUMERIC   0x00000001 
#define VST_FLOATING  0x00000002
#define VST_TEXT      0x00000004
#define VST_BINARY    0x00000008
#define VST_VARLEN    0x00000010

#define vst_is_numeric(d)   (d->flags & VST_NUMERIC)
#define vst_is_floating(d)  (d->flags & VST_FLOATING)
#define vst_is_text(d)      (d->flags & VST_TEXT)
#define vst_is_binary(d)    (d->flags & VST_BINARY)
#define vst_is_varlen(d)    (d->flags & VST_VARLEN)

typedef struct _tag_type_desc {
  unsigned int id;
  const char *name;
  unsigned int length;

  unsigned int flags;
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
