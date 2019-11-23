#include "unpack.h"
#include "auunpack.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

unsigned int unpack(filelist *flist, unsigned int i) {
    char *unpackedfile;
    char *ext;
    unsigned int ret;
    if(flist->files[i].packedfile == NULL) return 1;

    unpackedfile = strdup(flist->files[i].packedfile);
    if(unpackedfile == NULL) return 1;
    ext = strrchr(unpackedfile,'.');
    if(ext == NULL) {
        free(unpackedfile);
        return 1;
    }
    strcpy(ext,".au");
    ret = auunpack(flist->files[i].packedfile,unpackedfile);
    if(ret == 0) {
        util_delfile(flist->files[i].packedfile);
    } else {
        util_delfile(unpackedfile);
    }
    free(unpackedfile);
    return ret;

}
