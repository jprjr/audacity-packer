#ifndef PROBE_H
#define PROBE_H

#include "filelist.h"

#ifdef __cplusplus
extern "C" {
#endif

int probe(const char *path, filelist *flist);

#ifdef __cplusplus
}
#endif

#endif
