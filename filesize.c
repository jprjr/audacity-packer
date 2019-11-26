#if defined(_WIN32) || defined(_WIN64)
#ifndef _UNICODE
#define _UNICODE 1
#endif
#ifndef UNICODE
#define UNICODE 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <stdlib.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "filesize.h"
#include "winstr.h"

size_t filesize(const char *filename) {
#if defined(_WIN32) || defined(_WIN64)
    wchar_t *w_path;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    LARGE_INTEGER size;

    w_path = mb_to_w(filename);
    if(w_path == NULL) return 0;

    if(!GetFileAttributesExW(w_path,GetFileExInfoStandard,&fad)) return 0;
    free(w_path);

    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
#else
    struct stat st;
    if(stat(filename,&st) != 0) return 0;
    return st.st_size;
#endif

}
