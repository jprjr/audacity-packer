#include "recfind.h"

#if defined(_WIN32) || defined(_WIN64)
#ifndef _UNICODE
#define _UNICODE 1
#endif
#ifndef UNICODE
#define UNICODE 1
#endif
#endif

#include "tinydir.h"
#include "winstr.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define FOLDER_SEP "\\"
#define CHAR wchar_t
#else
#define FOLDER_SEP "/"
#define CHAR char
#endif

#define DOT "."

#include "filelist.h"
#include <stdio.h>

void recfind(const char *folder, filelist *flist) {
    tinydir_dir dir;
    tinydir_file file;
    fileinfo *info;
    CHAR *w_path;
    char *mb_file;
    char *subfile;

    info = NULL;
    w_path = NULL;
    mb_file = NULL;
    subfile = NULL;

    w_path = mb_to_w(folder);
    if(w_path == NULL) return;

    tinydir_open(&dir,w_path);
    while(dir.has_next) {
        tinydir_readfile(&dir,&file);

        mb_file = w_to_mb(file.name);
        if(mb_file == NULL) goto next;

        if(strncmp(mb_file,DOT,1) == 0) {
            goto next;
        }

        subfile = (char *)malloc(
          sizeof(char) * (
          strlen(folder) +
          strlen(FOLDER_SEP) +
          strlen(mb_file) + 1));
        if(subfile == NULL) goto next;
        memset(subfile,0,sizeof(char));
        strcpy(subfile,folder);
        strcat(subfile,FOLDER_SEP);
        strcat(subfile,mb_file);

        if(file.is_reg) {
            info = filelist_update(flist,mb_file,subfile);
            (void)info;
        }
        else if(file.is_dir) {
            recfind(subfile,flist);
        }
        next:
        if(mb_file != NULL) {
            free(mb_file);
            mb_file = NULL;
        }

        if(subfile != NULL) {
            free(subfile);
            subfile = NULL;
        }
        tinydir_next(&dir);
    }
    tinydir_close(&dir);
    free(w_path);
}

