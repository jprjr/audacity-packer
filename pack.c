#include "pack.h"
#include "aupack.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

unsigned int pack(filelist *flist, unsigned int i) {
    char *packedfile;
    char *ext;
    unsigned int ret;

    if(flist->files[i].sourcefile == NULL) {
        DEBUG_LOG("File %u is null, skipping\n",i);
        return 0;
    }

    packedfile = strdup(flist->files[i].sourcefile);
    if(packedfile == NULL) {
        DEBUG_LOG("Out of memory\n");
        return 1;
    }

    ext = strrchr(packedfile,'.');
    if(ext == NULL) {
        DEBUG_LOG("Could not find extension in %s\n",packedfile);
        free(packedfile);
        return 1;
    }

    *ext = '\0';

    packedfile = realloc(packedfile,strlen(packedfile) + 4);
    if(packedfile == NULL) {
        DEBUG_LOG("Could not realloc packedfile\n");
        return 1;
    }

    strcat(packedfile,".wv");
    ret = aupack(flist->files[i].sourcefile,packedfile,flist->files[i].samplerate);
    if(ret == 0) {
        DEBUG_LOG("Removing %s\n",flist->files[i].sourcefile);
        util_delfile(flist->files[i].sourcefile);
    } else {
        DEBUG_LOG("Removing %s\n",flist->files[i].packedfile);
        util_delfile(packedfile);
    }
    free(packedfile);
    return ret;

}
