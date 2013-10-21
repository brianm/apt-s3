/*
 * jsonz.h
 * libjsonz
 *
 * Created by Árpád Goretity on 29/12/2011.
 * Licensed under a CreativeCommons Attribution 3.0 Unported License
 */

#include <string.h>
#include <stdlib.h>
#include <jsonz/object.h>

#ifndef JSONZ_JSONZ_H
#define JSONZ_JSONZ_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void *jsonz_parse(const char *str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* JSONZ_JSONZ_H */

