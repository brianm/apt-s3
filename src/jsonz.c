/*
 * jsonz.c
 * libjsonz
 *
 * Created by Árpád Goretity on 29/12/2011.
 * Licensed under a CreativeCommons Attribution 3.0 Unported License
 */

#include <ctype.h>
#include <jsonz/jsonz.h>

typedef struct jsonz_parser_t {
	const char *str;
} jsonz_parser_t;

static inline void skip_space(jsonz_parser_t *p)
{
	while (isspace(*p->str))
		p->str++;
}

static long unescape_char(jsonz_parser_t *p)
{
	/* skip leading backslash '\' */
	switch (*++p->str) {
	case '\\': p->str++; return '\\';
	case '/':  p->str++; return  '/';
	case '\'': p->str++; return '\'';
	case '"':  p->str++; return '\"';
	case 'a':  p->str++; return '\a';
	case 'b':  p->str++; return '\b';
	case 'f':  p->str++; return '\f';
	case 'n':  p->str++; return '\n';
	case 'r':  p->str++; return '\r';
	case 't':  p->str++; return '\t';
	case '0':  p->str++; return '\0';
	case 'u':
		p->str++;
		if (isxdigit(p->str[0])
		 && isxdigit(p->str[1])
		 && isxdigit(p->str[2])
		 && isxdigit(p->str[3])) {
			char buf[5];
			memcpy(buf, p->str, 4);
			buf[4] = 0;
			p->str += 4;
			return strtol(buf, NULL, 16); /* can't overflow */
		}

		/* invalid hex escape sequence */
		fprintf(
			stderr,
			"libjsonz: invalid escape sequence '\\u%c%c%c%c'\n",
			p->str[0],
			p->str[1],
			p->str[2],
			p->str[3]
		);
		return -1;
	default:
		fprintf(stderr, "libjsonz: invalid escape sequence \\%c\n", *p->str);
		return -1;
	}
}

static void *parse(jsonz_parser_t *p);

static void *parse_num(jsonz_parser_t *p)
{
	int isfloat = 0;
	
	const char *s = p->str;
	while (isdigit(*p->str))
		p->str++;
	
	if (*p->str == '.') {
		isfloat = 1;
		p->str++;
	}
	
	while (isdigit(*p->str))
		p->str++;
	
	if (tolower(*p->str) == 'e') {
		isfloat = 1;
		p->str++;

		if (!isdigit(*p->str))
			return NULL;
	
		while (isdigit(*p->str))
			p->str++;
	}
	
	char *end;
	void *obj;
	if (isfloat) {
		double d = strtod(s, &end);
		if (end != p->str)
			return NULL;
		
		obj = jsonz_object_new(JSONZ_TYPE_FLOAT);
		jsonz_number_set_float_value(obj, d);
	} else {
		long long n = strtoll(s, &end, 10);
		if (end != p->str)
			return NULL;
		
		obj = jsonz_object_new(JSONZ_TYPE_INT);
		jsonz_number_set_int_value(obj, n);
	}

	return obj;
}

static void *parse_str(jsonz_parser_t *p)
{
	size_t sz = 0x20;
	size_t n = 0;
	char *buf = malloc(sz);
	if (buf == NULL)
		return NULL;

	/* skip '"' */
	p->str++;
	
	while (*p->str != '"') {
		if (*p->str == 0) {
			/* premature end of string */
			fprintf(stderr, "end of input before closing \" in string\n");
			free(buf);
			return NULL;
		}
		
		if (p->str[0] == '\\') {
			long c = unescape_char(p);
			if (c < 0) {
				/* error unescaping the character */
				free(buf);
				return NULL;
			}
			
			if (c >> 8) {
				buf[n++] = (c & 0xff00) >> 8;
			}
			
			buf[n++] = c & 0xff;
		} else {
			buf[n++] = *p->str++;
		}
		
		/* expand the buffer if necessary */
		if (n >= sz) {
			sz <<= 1;
			char *tmp = realloc(buf, sz);
			if (tmp == NULL)
				abort();
			
			buf = tmp;
		}
		
		
	}
	
	buf[n] = 0;
	
	/* skip '"' */
	p->str++;
	
	void *obj = jsonz_object_new(JSONZ_TYPE_STRING);
	jsonz_string_set_str_nocopy(obj, buf);
	return obj;
}

static void *parse_tru(jsonz_parser_t *p)
{
	if (memcmp(p->str, "true", 4) == 0) {
		p->str += 4;
		void *obj = jsonz_object_new(JSONZ_TYPE_BOOL);
		jsonz_number_set_bool_value(obj, 1);
		return obj;
	}
	
	fprintf(stderr, "libjsonz: expected 'true'\n");
	return NULL;
}

static void *parse_fal(jsonz_parser_t *p)
{
	if (memcmp(p->str, "false", 5) == 0) {
		p->str += 5;
		void *obj = jsonz_object_new(JSONZ_TYPE_BOOL);
		jsonz_number_set_bool_value(obj, 0);
		return obj;
	}
	
	fprintf(stderr, "libjsonz: expected 'false'\n");
	return NULL;
}

static void *parse_nul(jsonz_parser_t *p)
{
	if (memcmp(p->str, "null", 4) == 0) {
		p->str += 4;
		return jsonz_object_new(JSONZ_TYPE_NULL);
	}
	
	fprintf(stderr, "libjsonz: expected 'null'\n");
	return NULL;
}

static void *parse_arr(jsonz_parser_t *p)
{
	/* skip '[' */
	p->str++;
	
	skip_space(p);
	if (*p->str == 0) {
		fprintf(stderr, "libjsonz: unexpected end of input in array\n");
		return NULL;
	}
	
	void *obj = jsonz_object_new(JSONZ_TYPE_ARRAY);
	
	while (*p->str != ']') {
		void *elem = parse(p);
		
		if (elem == NULL) {
			jsonz_object_free(obj);
			return NULL;
		}

		skip_space(p);
		if (*p->str != ',' && *p->str != ']') {
			fprintf(stderr, "libjsonz: expected ']' or ',' in array\n");
			jsonz_object_free(elem);
			jsonz_object_free(obj);
			return NULL;
		}
		
		jsonz_array_add(obj, elem);

		/* skip ',' */
		if (*p->str == ',')
			p->str++;
	}
	
	/* skip ']' */
	p->str++;
	return obj;
}

static void *parse_dic(jsonz_parser_t *p)
{
	/* skip '{' */
	p->str++;
	
	skip_space(p);
	if (*p->str == 0) {
		fprintf(stderr, "libjsonz: unexpected end of input in dictionary\n");
		return NULL;
	}
	
	void *obj = jsonz_object_new(JSONZ_TYPE_DICT);
	
	while (*p->str != '}') {
		void *key = parse(p);
		
		if (key == NULL) {
			jsonz_object_free(obj);
			return NULL;
		}
		
		if (jsonz_object_get_type(key) != JSONZ_TYPE_STRING) {
			fprintf(stderr, "libjsonz: dictionary key must be a string\n");
			jsonz_object_free(key);
			jsonz_object_free(obj);
			return NULL;
		}
		
		skip_space(p);
		if (*p->str != ':') {
			fprintf(stderr, "libjsonz: expected ':' after dictionary key\n");
			jsonz_object_free(key);
			jsonz_object_free(obj);
			return NULL;	
		}

		/* skip ':' */
		p->str++;
		skip_space(p);

		void *elem = parse(p);
		if (elem == NULL) {
			fprintf(stderr, "libjsonz: expected value after key in dictionary\n");
			jsonz_object_free(key);
			jsonz_object_free(elem);
			jsonz_object_free(obj);
			return NULL;	
		}
		
		skip_space(p);
		if (*p->str != ',' && *p->str != '}') {
			fprintf(stderr, "libjsonz: expected '}' or ',' in dictionary\n");
			jsonz_object_free(key);
			jsonz_object_free(elem);
			jsonz_object_free(obj);
			return NULL;
		}
		
		jsonz_dict_set(obj, jsonz_string_get_str(key), elem);
		
		skip_space(p);
		
		/* skip ',' */
		if (*p->str == ',')
			p->str++;
	}
	
	/* skip '}' */
	p->str++;
	return obj;
}

void *jsonz_parse(const char *str)
{
	jsonz_parser_t p = { str };
	void *obj = parse(&p);
	return obj;
}

/* a simple recursive descent parser */
static void *parse(jsonz_parser_t *p)
{
	skip_space(p);
	
	switch (*p->str) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': return parse_num(p);
	case '"': return parse_str(p);
	case '[': return parse_arr(p);
	case '{': return parse_dic(p);
	case 't': return parse_tru(p);
	case 'f': return parse_fal(p);
	case 'n': return parse_nul(p);
	default : return NULL;
	}
}

