#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

int util_isdir(const char *path);
int util_isfile(const char *path);
int util_delfile(const char *path);

#ifdef __cplusplus
}
#endif

#endif
