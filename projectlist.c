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
#define FOLDER_SEP "\\"
#define CHAR wchar_t
#else
#define FOLDER_SEP "/"
#define CHAR char
#endif
#define DOT "."
#define EXT "aup"

#include "projectlist.h"
#include "tinydir.h"
#include "winstr.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

projectlist *projectlist_new(void) {
    projectlist *plist;
    plist = (projectlist *)malloc(sizeof(projectlist));
    if(plist != NULL) {
        plist->projects = NULL;
        plist->total = 0;
    }
    return plist;
}

void projectlist_free(projectlist *plist) {
    unsigned int i = 0;
    if(plist->projects != NULL) {
        for(i=0;i<plist->total;i++) {
            if(plist->projects[i] != NULL) free(plist->projects[i]);
        }
        free(plist->projects);
    }
    free(plist);
}

void projectlist_append(projectlist *plist, const char *path) {
    plist->projects = (char **)realloc(plist->projects,sizeof(char *) * (plist->total + 1));
    if(plist->projects == NULL) return;
    plist->projects[plist->total] = strdup(path);
    if(plist->projects[plist->total] == NULL) return;
    plist->total++;
}


int projectlist_fill(projectlist *plist, const char *path) {
    tinydir_dir dir;
    tinydir_file file;
    CHAR *w_path;
    char *mb_file;
    char *mb_ext;
    char *subfile;
    w_path = NULL;
    mb_file = NULL;
    mb_ext = NULL;
    subfile = NULL;

    w_path = mb_to_w(path);
    if(w_path == NULL) return 1;

    tinydir_open(&dir,w_path);
    while(dir.has_next) {
        tinydir_readfile(&dir,&file);

        mb_file = w_to_mb(file.name);
        if(mb_file == NULL) goto next;

        mb_ext = w_to_mb(file.extension);
        if(mb_ext == NULL) goto next;

        if(strncmp(mb_file,DOT,1) == 0) {
            goto next;
        }
        subfile = (char *)malloc(
          sizeof(char) * (
          strlen(path) +
          strlen(FOLDER_SEP) +
          strlen(mb_file) + 1));
        if(subfile == NULL) goto next;
        memset(subfile,0,sizeof(char));
        strcpy(subfile,path);
        strcat(subfile,FOLDER_SEP);
        strcat(subfile,mb_file);
        if(file.is_reg) {
            if(strcmp(mb_ext,EXT) == 0) {
                projectlist_append(plist,subfile);
            }
        }
        else if(file.is_dir) {
            projectlist_fill(plist,subfile);
        }

        next:
        if(subfile != NULL) {
            free(subfile);
            subfile = NULL;
        }
        if(mb_file != NULL) {
            free(mb_file);
            mb_file = NULL;
        }
        if(mb_ext != NULL) {
            free(mb_ext);
            mb_ext = NULL;
        }
        tinydir_next(&dir);
    }
    tinydir_close(&dir);
    free(w_path);
    return 0;
}
