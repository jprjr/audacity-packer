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
#define FOLDER_SEP "\\"
#else
#include <libgen.h>
#define FOLDER_SEP "/"
#endif

#include "probe.h"
#include "recfind.h"
#include "filelist.h"
#include "audparse.h"
#include <stdlib.h>
#include <string.h>


int probe(const char *_path, filelist *flist) {
    int ret;
    char *folder;
    char *path;
#if defined(_WIN32) || defined(_WIN64)
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    unsigned int i = 0;
#else
    char *dir;
#endif
    path = NULL;
    ret = 0;
    folder = NULL;

    path = strdup(_path);
    if(path == NULL) {
        ret = -1;
        goto cleanup;
    }

#if defined(_WIN32) || defined(_WIN64)
    for(i=0;i<strlen(path);i++) {
        if(path[i] == '/') path[i] = '\\';
    }
    _splitpath(path,drive,dir,fname,ext);

    folder = (char *)malloc(sizeof(char) * (
      strlen(drive) +
      strlen(dir) +
      1));
    if(folder ==  NULL) {
        ret = -1;
        goto cleanup;
    }
    folder[0] = 0;

    strcpy(folder,drive);
    strcat(folder,dir);
#else
    dir = dirname(path);
    folder = strdup(dir);
    if(folder == NULL) {
        ret = -1;
        goto cleanup;
    }
#endif

#if defined(_WIN32) || defined(_WIN64)
    folder = realloc(folder,sizeof(char) * (
      strlen(folder) +
      strlen(flist->root)
      + 1));
#else
    folder = realloc(folder,sizeof(char) * (
      strlen(folder) +
      1 +
      strlen(flist->root)
      + 1));
#endif
    if(folder == NULL) {
        ret = -1;
        goto cleanup;
    }
#if !(defined(_WIN32) || defined(_WIN64))
    strcat(folder,FOLDER_SEP);
#endif
    strcat(folder,flist->root);
    recfind(folder,flist);

cleanup:
    if(path != NULL) free(path);
    if(folder != NULL) free(folder);

    return ret;
}
