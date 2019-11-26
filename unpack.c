#include "unpack.h"
#include "auunpack.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

unsigned int unpack(filelist *flist, unsigned int i) {
    char *unpackedfile;
    char *ext;
    unsigned int ret;

    if(flist->files[i].packedfile == NULL) {
        DEBUG_LOG("File %u is null, skipping\n",i);
        return 0;
    }

    unpackedfile = strdup(flist->files[i].packedfile);
    if(unpackedfile == NULL) {
        DEBUG_LOG("Out of memory\n");
        return 1;
    }

    ext = strrchr(unpackedfile,'.');
    if(ext == NULL) {
        DEBUG_LOG("Could not find extension in %s",unpackedfile);
        free(unpackedfile);
        return 1;
    }

    *ext = '\0';

    unpackedfile = realloc(unpackedfile,strlen(unpackedfile) + 4);
    if(unpackedfile == NULL) {
        DEBUG_LOG("Could not realloc unpackedfile\n");
        return 1;
    }

    strcat(unpackedfile,".au");
    ret = auunpack(flist->files[i].packedfile,unpackedfile);
    if(ret == 0) {
        DEBUG_LOG("Removing %s\n",flist->files[i].packedfile);
        util_delfile(flist->files[i].packedfile);
    } else {
        DEBUG_LOG("Removing %s\n",unpackedfile);
        util_delfile(unpackedfile);
    }
    free(unpackedfile);
    return ret;

}
