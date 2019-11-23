#ifndef WINSTR_H
#define WINSTR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
char *w_to_mb(const wchar_t *);
wchar_t *mb_to_w(const char *);
#else
char *w_to_mb(const char *);
char *mb_to_w(const char *);
#endif

#ifdef __cplusplus
}
#endif

#endif
