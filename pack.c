#include "pack.h"
#include "aupack.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

unsigned int pack(filelist *flist, unsigned int i) {
    char *packedfile;
    char *ext;
    unsigned int ret;
    packedfile = strdup(flist->files[i].sourcefile);
    if(packedfile == NULL) return 1;
    ext = strrchr(packedfile,'.');
    if(ext == NULL) {
        free(packedfile);
        return 1;
    }
    strcpy(ext,".wv");
    ret = aupack(flist->files[i].sourcefile,packedfile,flist->files[i].samplerate);
    if(ret == 0) {
        util_delfile(flist->files[i].sourcefile);
    } else {
        util_delfile(packedfile);
    }
    free(packedfile);
    return ret;

}
