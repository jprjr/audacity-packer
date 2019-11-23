#if defined(__linux__)
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define FSEEK(a,b,c) _fseeki64(a,b,c)
#define FTELL(a) _ftelli64(a)
#else
#define FSEEK(a,b,c) fseeko(a,b,c)
#define FTELL(a) ftello(a)
#endif

#include "filesize.h"
#include "fopen_wrapper.h"
#include <stdlib.h>
#include <stdio.h>

size_t filesize(const char *filename) {
    FILE *f;
    size_t length;

    f = NULL;
    length = 0;

    f = fopen_wrapper(filename,"rb");
    if(f == NULL) goto cleanup;
    if(FSEEK(f,0,SEEK_END) != 0) goto cleanup;
    length = (size_t)FTELL(f);
cleanup:
    if(f != NULL) fclose(f);
    return length;
}
