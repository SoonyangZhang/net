/**
 * Copyright (c) 2016 rxi
 *
 * from https://github.com/rxi/ini
 *
 */

#pragma once
#ifdef __cplusplus
extern "C"  {
#endif
#define INI_VERSION "0.1.1"
typedef struct ini_t ini_t;

ini_t*      ini_load(const char *filename);
void        ini_free(ini_t *ini);
const char* ini_get(ini_t *ini, const char *section, const char *key);
int         ini_sget(ini_t *ini, const char *section, const char *key, const char *scanfmt, void *dst);
#ifdef  __cplusplus
}
#endif
