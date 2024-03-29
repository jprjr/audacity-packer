#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
#include "winstr.h"

int util_isdir(const char *path) {
    DWORD dwAttrs;
    wchar_t *w_path;
    int ret;
    w_path = mb_to_w(path);
    if(w_path == NULL) return -1;

    dwAttrs = GetFileAttributesW(w_path);
    if(dwAttrs == INVALID_FILE_ATTRIBUTES) {
        free(w_path);
        return -1;
    }
    ret = (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    free(w_path);
    return ret;
}

int util_isfile(const char *path) {
    DWORD dwAttrs;
    wchar_t *w_path;
    int ret;
    w_path = mb_to_w(path);
    if(w_path == NULL) return -1;

    dwAttrs = GetFileAttributesW(w_path);
    if(dwAttrs == INVALID_FILE_ATTRIBUTES) {
        free(w_path);
        return -1;
    }
    ret = (dwAttrs & FILE_ATTRIBUTE_NORMAL) ? 1 : 0;
    free(w_path);
    return ret;
}

int util_delfile(const char *path) {
    wchar_t *w_path;
    int ret;
    w_path = mb_to_w(path);
    if(w_path == NULL) return -1;
    ret = DeleteFileW(w_path) != 0;
    free(w_path);
    return ret;
}

#else
#include <unistd.h>
#include <sys/stat.h>
int util_isdir(const char *path) {
    struct stat st;
    if(stat(path,&st) == 0) {
        return S_ISDIR(st.st_mode);
    }

    return -1;
}

int util_isfile(const char *path) {
    struct stat st;
    if(stat(path,&st) == 0) {
        return S_ISREG(st.st_mode);
    }

    return -1;
}

int util_delfile(const char *path) {
    return unlink(path) == 0;
}
#endif

static const char *hsize_formats[] = {
    "B",
    "KB",
    "MB",
    "GB",
    "TB",
    NULL
};

char *util_hsize(uint64_t filesize) {
    char *fmt = NULL;
    unsigned int size_len = 0;
    unsigned int i = 0;
    double hsize = (double)filesize;

    while(hsize > 1024.0f && hsize_formats[i+1] != NULL) {
        hsize /= 1024.0f;
        i++;
    }

    size_len = snprintf(NULL,0,"%0.3f %s",hsize,hsize_formats[i]);
    fmt = (char *)malloc(size_len+1);
    if(fmt == NULL) return NULL;
    snprintf(fmt,size_len+1,"%0.3f %s",hsize,hsize_formats[i]);
    return fmt;
}
