#ifndef AUDPARSE_H
#define AUDPARSE_H

#include "filelist.h"
#include "yxml.h"
#include <stdint.h>

struct aud_parser_s {
    yxml_t *x;
    uint8_t *buffer;
    uint8_t *xml;
    char databuf[100];
    size_t datapos;
    size_t xmlpos;
    size_t xmlsize;
    unsigned int rate;
};

typedef struct aud_parser_s aud_parser;

#ifdef __cplusplus
extern "C" {
#endif

aud_parser *aud_new(void);
void aud_free(aud_parser *parser);

/* return 0 on success, -1 on error */
int aud_load(aud_parser *parser, const char *filename);
void aud_unload(aud_parser *parser);

/* return -2 on pcm warning, -1 on error, 0 on success, 1 on complete */
int aud_parse(aud_parser *parser, filelist *flist);


#ifdef __cplusplus
}
#endif

#endif
