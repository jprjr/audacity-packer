#ifndef FOPEN_WRAPPER_H
#define FOPEN_WRAPPER_H
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE *fopen_wrapper(const char *filename, const char *mode);

#ifdef __cplusplus
}
#endif

#endif
