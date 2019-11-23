#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "audparse.h"
#include "filelist.h"
#include "slurp.h"
#include "yxml.h"


long audparse(const char *filename,const char *foldername,filelist *flist) {
    yxml_t *x;
    uint8_t *buffer;
    uint8_t *xml;
    uint8_t *xmlp;
    size_t filesize = 0;
    unsigned int ret = 0;
    unsigned long rate = 0;
    char databuf[100];
    int datapos = 0;
    yxml_ret_t y = YXML_OK;

    x = NULL;
    buffer = NULL;
    xml = NULL;

    x = (yxml_t *)malloc(sizeof(yxml_t));
    if(x == NULL) {
        ret = -1;
        goto error;
    }


    xml = slurp(filename,&filesize);
    if(xml == NULL) {
        ret = -1;
        goto error;
    }
    xmlp = xml;

    buffer = (uint8_t *)malloc(8192);
    if(buffer == NULL) {
        ret = -1;
        goto error;
    }
    yxml_init(x,buffer,8192);

    while((unsigned long)(xmlp - xml) < filesize) {
        y = yxml_parse(x, *xmlp++);
        if(y < 0) {
            ret = -1;
            goto error;
        }
        if(y == YXML_ELEMSTART) {
            if(strcmp(x->elem,"pcmaliasblockfile") == 0) {
                ret = -2;
                goto error;
            }
        }
        else if(y == YXML_ATTRSTART) {
            datapos = 0;
            databuf[datapos] = 0;
        } else if(y == YXML_ATTREND) {
            databuf[datapos] = 0;
            if(strcmp(x->elem,"wavetrack") == 0) {
              if(strcmp(x->attr,"rate") == 0) {
                  rate = strtoul(databuf,NULL,10);
              }
            }
            else if(strcmp(x->elem,"simpleblockfile") == 0) {
              if(strcmp(x->attr,"filename") == 0) {
                  if(filelist_append(flist,databuf,rate) == NULL) {
                      ret = -1;
                      goto error;
                  }
                  ret++;
              }
            }
            else if(strcmp(x->elem,"project") == 0) {
              if(strcmp(x->attr,"projname") == 0) {
                  if(filelist_root(flist,databuf) == NULL) {
                      ret = -1;
                      goto error;
                  }
              }
            }
        }
        else if(y == YXML_ATTRVAL) {
            databuf[datapos++] = x->data[0];
        }

    }

    goto cleanup;

error:
    switch(ret) {
        case -1: fprintf(stderr,"error: out of memory\n"); break;
        case -2: fprintf(stderr,"warning: pcmaliasblockfile found\n"); break;
    }

cleanup:
    if(x != NULL) free(x);
    if(buffer != NULL) free(buffer);
    if(xml != NULL) free(xml);
    return ret;
}
