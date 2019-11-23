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

wchar_t *mb_to_w(const char *src) {
    wchar_t *str;
    int width;
    str = NULL;

    width = MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      src,
      -1,
      NULL,
      0);

    if(width <= 0) {
        return NULL;
    }

    str = (wchar_t *)malloc(sizeof(wchar_t) * width);
    if(str == NULL) {
        return NULL;
    }

    width = MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      src,
      -1,
      str,
      width);
    if(width <= 0) {
        free(str);
        return NULL;
    }

    return str;
}

char *w_to_mb(const wchar_t *src) {
    char *str;
    int width;
    str = NULL;

    width = WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      src,
      -1,
      NULL,
      0,
      NULL,
      NULL);

    if(width <= 0) {
        return NULL;
    }

    str = malloc(sizeof(char) * width);
    if(str == NULL) {
        return NULL;
    }

    width = WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      src,
      -1,
      str,
      width,
      NULL,
      NULL);

    if(width <= 0) {
        free(str);
        return NULL;
    }

    return str;
}

#else
#include <string.h>
char *w_to_mb(const char *s) {
    return strdup(s);
}

char *mb_to_w(const char *s) {
    return strdup(s);
}

#endif
