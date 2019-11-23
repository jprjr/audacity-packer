#ifndef SLURP_H
#define SLURP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *slurp(const char *filename, size_t *filesize);

#ifdef __cplusplus
}
#endif

#endif
