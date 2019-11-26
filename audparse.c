#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "audparse.h"
#include "filelist.h"
#include "slurp.h"
#include "yxml.h"

aud_parser *aud_new(void) {
    aud_parser *parser = (aud_parser *)malloc(sizeof(aud_parser));
    if(parser == NULL) {
        return NULL;
    }
    parser->x = (yxml_t *)malloc(sizeof(yxml_t));
    if(parser->x == NULL) {
        aud_free(parser);
        return NULL;
    }
    parser->buffer = (uint8_t *)malloc(sizeof(uint8_t) * 8192);
    if(parser->buffer == NULL) {
        aud_free(parser);
        return NULL;
    }
    parser->xml = NULL;
    parser->xmlsize = 0;

    return parser;
}

void aud_unload(aud_parser *parser) {
    if(parser->xml != NULL) free(parser->xml);
    parser->xml = NULL;
    return;
}

void aud_free(aud_parser *parser) {
    if(parser != NULL) {
        aud_unload(parser);
        if(parser->x != NULL) {
            free(parser->x);
        }
        if(parser->buffer != NULL) {
            free(parser->buffer);
        }
        free(parser);
        parser = NULL;
    }
    return;
}

int aud_load(aud_parser *parser, const char *filename) {
    aud_unload(parser);
    yxml_init(parser->x,parser->buffer,8192);

    parser->xml = slurp(filename, &(parser->xmlsize));
    if(parser->xml == NULL) {
        parser->xmlsize = 0;
        return -1;
    }
    parser->xmlpos = 0;
    parser->datapos = 0;
    parser->databuf[0] = 0;
    return 0;
}

/* -1: unrecorerable error
 * -2: warning, found a pcmalias
 *  1: EOF
 *  0: normal
 */
int aud_parse(aud_parser *parser, filelist *flist) {
    yxml_ret_t y = YXML_OK;
    if(parser->xml == NULL) return -1;

    if(parser->xmlpos < parser->xmlsize) {
        y = yxml_parse(parser->x, parser->xml[parser->xmlpos++]);
    } else if(parser->xmlpos == parser->xmlsize) {
        y = yxml_eof(parser->x);
        parser->xmlpos++;
    } else {
        return 1;
    }
    if(y < 0) {
        return -1;
    }
    if(y == YXML_ELEMSTART) {
        if(strcmp(parser->x->elem,"pcmaliasblockfile") == 0) {
            return -1;
        }
    }
    else if(y == YXML_ATTRSTART) {
        parser->datapos = 0;
        parser->databuf[parser->datapos] = 0;
    } else if(y == YXML_ATTREND) {
        parser->databuf[parser->datapos] = 0;
        if(strcmp(parser->x->elem,"wavetrack") == 0) {
          if(strcmp(parser->x->attr,"rate") == 0) {
              parser->rate = strtoul(parser->databuf,NULL,10);
          }
        }
        else if(strcmp(parser->x->elem,"simpleblockfile") == 0) {
          if(strcmp(parser->x->attr,"filename") == 0) {
              if(filelist_append(flist,parser->databuf,parser->rate) == NULL) {
                  return -1;
              }
          }
        }
        else if(strcmp(parser->x->elem,"project") == 0) {
          if(strcmp(parser->x->attr,"projname") == 0) {
              if(filelist_root(flist,parser->databuf) == NULL) {
                  return -1;
              }
          }
        }
    }
    else if(y == YXML_ATTRVAL) {
        parser->databuf[parser->datapos++] = parser->x->data[0];
    }
    return 0;
}


