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
#define MAIN_SIG wmain(int argc, wchar_t *argv[], wchar_t *envp[])
#else
#define MAIN_SIG main(int argc, char *argv[], char *envp[])
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "cli.h"
#include "winstr.h"

int MAIN_SIG {
    (void)envp;
    int ret = 0;
    int i = 0;
    char **newargv;
    newargv = (char **)malloc(sizeof(char *) * argc);
    if(newargv == NULL) return 1;

#if defined(_WIN32) || defined(_WIN64)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    for(i = 0; i < argc; i++) {
        newargv[i] = w_to_mb(argv[i]);
        if(newargv[i] == NULL) return 1;
    }
    ret = 0;

    if(argc < 2) {
        ret = start_gui(argc,newargv);
        if(ret != -1) {
            for(i = 0; i < argc; i++) {
                free(newargv[i]);
            }
            free(newargv);
            return ret;
        }
    }
    ret = start_cli(argc,newargv);
    for(i = 0; i < argc; i++) {
        free(newargv[i]);
    }
    free(newargv);
    return ret;
}

