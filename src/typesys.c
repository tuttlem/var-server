#include "../include/typesys.h"

type_desc type_desc_table[] = {
  { 0x0000, "null",   0, 0, 0, 0, 0, 0 },
  { 0x0001, "bit",    1, 1, 0, 0, 0, 0 },
  { 0x0002, "int8",   1, 1, 0, 0, 0, 0 },
  { 0x0003, "int16",  2, 1, 0, 0, 0, 0 },
  { 0x0004, "int32",  4, 1, 0, 0, 0, 0 },
  { 0x0005, "int64",  8, 1, 0, 0, 0, 0 },
  { 0x0010, "float4", 4, 0, 1, 0, 0, 0 },
  { 0x0011, "float8", 8, 0, 1, 0, 0, 0 },
  { 0x0020, "text",   0, 0, 0, 1, 0, 1 },
};

int type_desc_table_len = 9;

type_desc* lookup_type(unsigned int type_id) {
  int i;
  type_desc *res = NULL;

  for (i = 0; i < type_desc_table_len && res == NULL; i ++) {
    if (type_desc_table[i].id == type_id) {
      res = &type_desc_table[i];
    }
  }

  return res;
}

type_desc* lookup_type_by_name(char *name) {
  int i;
  type_desc *res = NULL;

  for (i = 0; i < type_desc_table_len && res == NULL; i ++) {
    if (strcmp(type_desc_table[i].name, name) == 0) {
      res = &type_desc_table[i];
    }
  }

  return res;
}


/**
 * Prints the value inside of a container
 */
int vsval_print(vsval *v) {
  if (!v) {
    return ERR_INVPTR;
  }

  type_desc *desc = lookup_type(v->type_id);

  if (desc == NULL) {
    return ERR_INVTYPE;
  }

  if (desc->id == 0x0000) {
    printf("null");
  } else if (desc->is_numeric) {
    switch (desc->length) {
      case 1:
        printf("%i", *(char*)v->data);
        break;
      case 2:
        printf("%i", *(short *)v->data);
        break;
      case 4:
        printf("%i", *(int *)v->data);
        break;
      case 8:
        printf("%li", *(long *)v->data);
        break;
      default:
        return ERR_INVTYPE;
    }
    
  } else if (desc->is_floating) {
    if (desc->length == 4) {
      printf("%f", *(float *)v->data);
    } else if (desc->length == 8) {
      printf("%f", *(double *)v->data);
    } else {
      return ERR_INVTYPE;
    }
  } else if (desc->is_text) {
    printf("%s", (char *)v->data);
  } else if (desc->is_binary) {
    int idx = 0;
    for (idx = 0; idx < v->length; idx ++) {
      unsigned char c = ((unsigned char*)v->data)[idx];
      printf("%x ", c);
    }
  } else {
    return ERR_INVTYPE;
  }

  return ERR_SUCCESS;
}

/**
 * Creates a value container
 */
int vsval_create(char *name, vsval **v) {
  vsval *newval = NULL;
  type_desc *desc = lookup_type_by_name(name);

  if (desc == NULL || desc->id == 0) {
    return ERR_INVTYPE;
  }

  newval = (vsval *)malloc(sizeof(vsval));
  newval->type_id = desc->id;

  if (desc->is_var_length) {
    newval->data = malloc(VSVAL_DEFAULT_LENGTH);
    newval->length = VSVAL_DEFAULT_LENGTH;
  } else {
    newval->data = malloc(desc->length);
    newval->length = desc->length;
  }
    
  memset(newval->data, 0, newval->length);
  *v = newval;

  return ERR_SUCCESS;
}

/**
 * Destroys a value container
 */
int vsval_destroy(vsval **v) {
  if (!v) {
    return ERR_INVPTR;
  }

  if ((*v)->data) {
    free((*v)->data);
  }

  free(*v);
  *v = NULL;

  return ERR_SUCCESS;
}

/**
 * Sets the internal value of a value container
 */
int vsval_set(vsval *v, unsigned int type_id, void *data, unsigned int length) {
  if (!v) {
    return ERR_INVPTR;
  }

  type_desc *desc = lookup_type(type_id);

  if (desc == NULL || type_id == 0) {
    return ERR_INVTYPE;
  }

  int actual_len = desc->length ? desc->length : length;

  if (v->length != actual_len) {
    v->data = realloc(v->data, actual_len);
    v->length = actual_len;
  }

  v->type_id = type_id;
  memcpy(v->data, data, v->length);

  return ERR_SUCCESS;
}

/**
 * Sets the internal value of a value container to symbolic NULL
 */
int vsval_set_null(vsval *v) {
  if (!v) {
    return ERR_INVPTR;
  }

  if (v->data) {
    free(v->data);
    v->data = NULL;
  }

  v->length = 0;
  v->type_id = 0x0000;

  return ERR_SUCCESS;
}

int vsval_set_numeric(vsval *v, int i) {
  if (!v) {
    return ERR_INVPTR;
  }

  type_desc *desc = lookup_type(v->type_id);

  if (desc == NULL || v->type_id == 0 || !desc->is_numeric) {
    return ERR_INVTYPE;
  }

  memcpy(v->data, &i, desc->length);

  return ERR_SUCCESS;
}

int vsval_set_float(vsval *v, float f) {
  if (!v) {
    return ERR_INVPTR;
  }

  type_desc *desc = lookup_type(v->type_id);

  if (desc == NULL || v->type_id == 0 || 
      !desc->is_floating || desc->length != 4) {
    return ERR_INVTYPE;
  }

  memcpy(v->data, &f, desc->length);

  return ERR_SUCCESS;
}

int vsval_set_double(vsval *v, double f) {
  if (!v) {
    return ERR_INVPTR;
  }

  type_desc *desc = lookup_type(v->type_id);

  if (desc == NULL || v->type_id == 0 || 
      !desc->is_floating || desc->length != 8) {
    return ERR_INVTYPE;
  }

  memcpy(v->data, &f, desc->length);

  return ERR_SUCCESS;
}

int vsval_set_text(vsval *v, const char *s) {
  if (!v) {
    return ERR_INVPTR;
  }

  type_desc *desc = lookup_type(v->type_id);

  if (desc == NULL || v->type_id == 0 || !desc->is_text) {
    return ERR_INVTYPE;
  }

  if (v->length != strlen(s)) {
    v->length = strlen(s);
    v->data = realloc(v->data, v->length);
  }

  memcpy(v->data, s, v->length);

  return ERR_SUCCESS;
}
