/*
 * object.h
 * libjsonz
 *
 * Created by Árpád Goretity on 29/12/2011.
 * Licensed under a CreativeCommons Attribution 3.0 Unported License
 */


#ifndef JSONZ_OBJECT_H
#define JSONZ_OBJECT_H

#include <stdio.h>
#include <stdlib.h>


#define JSONZ_TYPE_NULL		0
#define JSONZ_TYPE_BOOL		1
#define JSONZ_TYPE_INT		2
#define JSONZ_TYPE_FLOAT	3
#define JSONZ_TYPE_STRING	4
#define JSONZ_TYPE_ARRAY	5
#define JSONZ_TYPE_DICT		6

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void *jsonz_object_new(int type);
void jsonz_object_free(void *obj);
int jsonz_object_get_type(void *obj);

int jsonz_number_get_bool_value(void *obj);
long long jsonz_number_get_int_value(void *obj);
double jsonz_number_get_float_value(void *obj);
void jsonz_number_set_bool_value(void *obj, int b);
void jsonz_number_set_int_value(void *obj, long long i);
void jsonz_number_set_float_value(void *obj, double d);

const char *jsonz_string_get_str(void *obj);
void jsonz_string_set_str(void *obj, const char *str_new);
void jsonz_string_set_str_nocopy(void *obj, char *str_new);

size_t jsonz_array_size(void *obj);
void *jsonz_array_get(void *obj, unsigned idx);
void jsonz_array_add(void *obj, void *elem);

size_t jsonz_dict_size(void *obj);
const char *jsonz_dict_key(void *obj, unsigned index);
void *jsonz_dict_get(void *obj, const char *key);
/* returns previous element for the key or NULL if not yet present */
void *jsonz_dict_set(void *obj, const char *key, void *elem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* JSONZ_OBJECT_H */

