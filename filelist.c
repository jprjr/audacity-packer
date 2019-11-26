#include <stdlib.h>
#include <string.h>
#include "filelist.h"
#include "filesize.h"
#include <stdio.h>

filelist *filelist_new(void) {
    filelist *flist;
    flist = (filelist *)malloc(sizeof(filelist));
    if(flist != NULL) {
        flist->files = NULL;
        flist->root = NULL;
        flist->total = 0;
    }
    return flist;
}

void filelist_free(filelist *flist) {
    unsigned int i;
    for(i=0;i<flist->total;i++) {
        if(flist->files[i].basename != NULL) free(flist->files[i].basename);
        if(flist->files[i].sourcefile != NULL) free(flist->files[i].sourcefile);
        if(flist->files[i].packedfile != NULL) free(flist->files[i].packedfile);
    }
    if(flist->root != NULL) free(flist->root);
    free(flist->files);
    free(flist);
}

char *filelist_root(filelist *flist, const char *root) {
    flist->root = strdup(root);
    return flist->root;
}

fileinfo *filelist_find(filelist *flist, const char *filename) {
    unsigned int i = 0;
    for(i=0;i<flist->total;i++) {
        if(strcmp(flist->files[i].basename,filename) == 0) {
            return &flist->files[i];
        }
    }
    return NULL;
}

fileinfo *filelist_update(filelist *flist, const char *filename, const char *fullpath) {
    fileinfo *info;
    char *basename;
    char *ext;

    info = NULL;
    basename = NULL;
    ext = NULL;

    basename = strdup(filename);
    if(basename == NULL) return NULL;
    ext = strrchr(basename,'.');
    if(ext == NULL) {
        goto fileinfo_update_cleanup;
    }
    ext[0] = 0;

    info = filelist_find(flist,basename);
    if(info == NULL) {
        goto fileinfo_update_cleanup;
    }

    if(strcmp(&ext[1],"au") == 0) {
        if(info->sourcefile == NULL) info->sourcefile = strdup(fullpath);
        info->sourcefile_size = filesize(fullpath);
        goto fileinfo_update_cleanup;

    }
    else if(strcmp(&ext[1],"wv") == 0) {
        if(info->packedfile == NULL) info->packedfile = strdup(fullpath);
        info->packedfile_size = filesize(fullpath);
        goto fileinfo_update_cleanup;

    }
    fileinfo_update_cleanup:
    if(basename != NULL) free(basename);
    return info;
}



fileinfo *filelist_append(filelist *flist, const char *filename, unsigned int samplerate) {
    fileinfo *info;
    char *basename;
    char *ext;
    basename = strdup(filename);
    if(basename == NULL) return NULL;
    ext = strrchr(basename,'.');
    if(ext == NULL) {
        free(basename);
        return NULL;
    }
    ext[0] = 0;

    info = filelist_find(flist,basename);
    if(info != NULL) {
        free(basename);
        info->samplerate = samplerate;
        return info;
    }

    flist->files = (fileinfo *)realloc(flist->files,(flist->total + 1) * sizeof(fileinfo));
    if(flist->files == NULL) return NULL;

    flist->files[flist->total].basename = NULL;
    flist->files[flist->total].sourcefile = NULL;
    flist->files[flist->total].packedfile = NULL;
    flist->files[flist->total].sourcefile_size = 0;
    flist->files[flist->total].packedfile_size = 0;

    info = &flist->files[flist->total];

    flist->files[flist->total].basename = strdup(basename);
    if(flist->files[flist->total].basename == NULL) {
        free(basename);
        return NULL;
    }

    flist->files[flist->total].samplerate = samplerate;
    flist->total++;
    free(basename);

    return &flist->files[flist->total - 1];
}

void filelist_totals(filelist *flist, uint64_t *total, uint64_t *source, uint64_t *packed, uint64_t *source_size, uint64_t *packed_size) {
    unsigned int i = 0;
    *total = flist->total;
    *source = 0;
    *packed = 0;
    *source_size = 0;
    *packed_size = 0;
    for(i=0;i<flist->total;i++) {
        if(flist->files[i].sourcefile != NULL) *source = *source + 1;
        if(flist->files[i].packedfile != NULL) *packed = *packed + 1;
        *source_size = *source_size + flist->files[i].sourcefile_size;
        *packed_size = *packed_size + flist->files[i].packedfile_size;
    }
}


