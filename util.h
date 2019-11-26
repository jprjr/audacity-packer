#ifndef UTIL_H
#define UTIL_H

#ifndef NDEBUG
#include <stdio.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define DEBUG_LOG(...)
#else
#define DEBUG_LOG(...) fprintf(stderr,__VA_ARGS__)
#endif

int util_isdir(const char *path);
int util_isfile(const char *path);
int util_delfile(const char *path);
char *util_hsize(uint64_t filesize);

#ifdef __cplusplus
}
#endif

#endif
