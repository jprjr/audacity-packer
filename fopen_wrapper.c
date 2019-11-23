#if defined(_WIN32) || defined(_WIN64)
#include "winstr.h"
#elif defined(__linux__)
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include "fopen_wrapper.h"


FILE *fopen_wrapper(const char *filename, const char *mode) {
#if defined(_WIN32) || defined(_WIN64)
    FILE *f;
    wchar_t *wide_filename;
    wchar_t *wide_mode;
    f = NULL;
    wide_filename = NULL;
    wide_mode = NULL;

    wide_filename = mb_to_w(filename);
    if(wide_filename == NULL) goto error;

    wide_mode = mb_to_w(mode);
    if(wide_mode == NULL) goto error;

    f = _wfopen(wide_filename,wide_mode);
    goto cleanup;
error:
    fprintf(stderr,"Error opening %s\n",filename);

cleanup:
    if(wide_filename != NULL) free(wide_filename);
    if(wide_mode != NULL) free(wide_mode);

    return f;

#else
    return fopen(filename,mode);
#endif
}
