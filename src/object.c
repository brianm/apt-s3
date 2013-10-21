/*
 * object.c
 * libjsonz
 *
 * Created by Árpád Goretity on 29/12/2011.
 * Licensed under a CreativeCommons Attribution 3.0 Unported License
 */

#include <stdlib.h>
#include <string.h>
#include <jsonz/object.h>

typedef struct jsonz_object_t {
	int type;
} jsonz_object_t;

typedef struct jsonz_null_t {
	jsonz_object_t base;
} jsonz_null_t;

typedef struct jsonz_number_t {
	jsonz_object_t base;
	union {
		double fltval;
		long long intval;
	} val;
} jsonz_number_t;

typedef struct jsonz_string_t {
	jsonz_object_t base;
	char *str;
} jsonz_string_t;

typedef struct jsonz_array_t {
	jsonz_object_t base;
	size_t size;
	void **vals;
} jsonz_array_t;

typedef struct jsonz_dict_t {
	jsonz_object_t base;
	size_t size;
	char **keys;
	void **vals;
} jsonz_dict_t;

static void *chkmalloc(size_t sz)
{
	void *p = malloc(sz);
	if (!p)
		abort();
	
	return p;
}

void *jsonz_object_new(int type)
{
	jsonz_object_t *obj;
	jsonz_number_t *num;
	jsonz_string_t *str;
	jsonz_array_t *arr;
	jsonz_dict_t *dic;
	void *o;
	
	switch (type) {
	case JSONZ_TYPE_NULL:
		o = chkmalloc(sizeof(jsonz_null_t));
		break;
	case JSONZ_TYPE_BOOL:
	case JSONZ_TYPE_INT:
		o = chkmalloc(sizeof(jsonz_number_t));
		num = o;
		num->val.intval = 0;
		break;
	case JSONZ_TYPE_FLOAT:
		o = chkmalloc(sizeof(jsonz_number_t));
		num = o;
		num->val.fltval = 0.0;
		break;
	case JSONZ_TYPE_STRING:
		o = chkmalloc(sizeof(jsonz_string_t));
		str = o;
		str->str = NULL;
		break;
	case JSONZ_TYPE_ARRAY:
		o = chkmalloc(sizeof(jsonz_array_t));
		arr = o;
		arr->size = 0;
		arr->vals = NULL;
		break;
	case JSONZ_TYPE_DICT:
		o = chkmalloc(sizeof(jsonz_dict_t));
		dic = o;
		dic->size = 0;
		dic->keys = NULL;
		dic->vals = NULL;
		break;
	default:
		return NULL;
	}
	
	obj = o;
	obj->type = type;
	return obj;
}

void jsonz_object_free(void *obj)
{
	jsonz_string_t *str = obj;
	jsonz_array_t *arr = obj;
	jsonz_dict_t *dic = obj;
	jsonz_object_t *o = obj;
	unsigned i;
	
	switch (o->type) {
	case JSONZ_TYPE_STRING:
		free(str->str);
		break;
	case JSONZ_TYPE_ARRAY:
		for (i = 0; i < arr->size; i++) {
			jsonz_object_free(arr->vals[i]);
		}
		break;
	case JSONZ_TYPE_DICT:
		for (i = 0; i < dic->size; i++) {
			free(dic->keys[i]);
			jsonz_object_free(dic->vals[i]);
		}
		break;
	case JSONZ_TYPE_NULL:
	case JSONZ_TYPE_BOOL:
	case JSONZ_TYPE_INT:
	case JSONZ_TYPE_FLOAT:
	default:
		break;
	}
	
	free(obj);
}

/*
 * Querying object types
 */

int jsonz_object_get_type(void *obj)
{
	jsonz_object_t *o = obj;
	return o->type;
}

/*
 * Number and boolean functions
 */

int jsonz_number_get_bool_value(void *obj)
{
	jsonz_number_t *num = obj;
	return num->val.intval != 0;
}

long long jsonz_number_get_int_value(void *obj)
{
	jsonz_number_t *num = obj;
	return num->val.intval;
}

double jsonz_number_get_float_value(void *obj)
{
	jsonz_number_t *num = obj;
	return num->val.fltval;
}

void jsonz_number_set_bool_value(void *obj, int b)
{
	jsonz_number_t *num = obj;
	num->val.intval = !!b;
}

void jsonz_number_set_int_value(void *obj, long long i)
{
	jsonz_number_t *num = obj;
	num->val.intval = i;
}

void jsonz_number_set_float_value(void *obj, double d)
{
	jsonz_number_t *num = obj;
	num->val.fltval = d;
}

/*
 * String functions
 */

const char *jsonz_string_get_str(void *obj)
{
	jsonz_string_t *str = obj;
	return str->str;
}

void jsonz_string_set_str(void *obj, const char *str_new)
{
	jsonz_string_t *str = obj;
	free(str->str);
	str->str = strdup(str_new);
}

void jsonz_string_set_str_nocopy(void *obj, char *str_new)
{
	jsonz_string_t *str = obj;
	free(str->str);
	str->str = str_new;
}

/*
 * Array functions
 */

size_t jsonz_array_size(void *obj)
{
	jsonz_array_t *arr = obj;
	return arr->size;
}

void *jsonz_array_get(void *obj, unsigned idx)
{
	jsonz_array_t *arr = obj;
	return arr->vals[idx];
}

void jsonz_array_add(void *obj, void *elem)
{
	jsonz_array_t *arr = obj;

	/*
	 * make room for the new element
	 */
	void **v = realloc(arr->vals, (arr->size + 1) * sizeof(*v));
	if (!v)
		abort();

	arr->vals = v;
	arr->vals[arr->size++] = elem;
}


/*
 * Dictionary functions (a. k. a. Associative Arrays)
 */

size_t jsonz_dict_size(void *obj)
{
	jsonz_dict_t *dic = obj;
	return dic->size;
}

const char *jsonz_dict_key(void *obj, unsigned index)
{
	
	jsonz_dict_t *dic = obj;
	return dic->keys[index];
}

void *jsonz_dict_get(void *obj, const char *key)
{
	jsonz_dict_t *dic = obj;
	unsigned i;
	for (i = 0; i < dic->size; i++) {
		if (strcmp(dic->keys[i], key) == 0)
			return dic->vals[i];
	}
	
	return NULL;
}

void *jsonz_dict_set(void *obj, const char *key, void *elem)
{
	/*
	 * if the object already contains this key, we just
	 * exchange it with the new one
	 */
	jsonz_dict_t *dic = obj;
	unsigned i;
	for (i = 0; i < dic->size; i++) {
		if (strcmp(dic->keys[i], key) == 0) {
			void *prv = dic->vals[i];
			dic->vals[i] = elem;
			return prv;
		}
	}
	
	/*
	 * if the key is not yet present, add it
	 */
	dic->size++;
	char **k = realloc(dic->keys, dic->size * sizeof(*k));
	void **v = realloc(dic->vals, dic->size * sizeof(*v));
	if (k == NULL || v == NULL)
		abort();
	
	dic->keys = k;
	dic->vals = v;
	char *key_new = strdup(key);
	if (!key_new)
		abort();
	
	dic->keys[dic->size - 1] = key_new;
	dic->vals[dic->size - 1] = elem;
	return NULL;
}

