#ifndef AUDPARSE_H
#define AUDPARSE_H

#include "filelist.h"

#ifdef __cplusplus
extern "C" {
#endif

long audparse(const char *filename,const char *foldername,filelist *flist);

#ifdef __cplusplus
}
#endif

#endif
