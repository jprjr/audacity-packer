#ifndef FILELIST_H
#define FILELIST_H

#include <stdint.h>

struct fileinfo_s {
    char *basename;
    char *sourcefile;
    char *packedfile;
    unsigned int samplerate;
    unsigned int sourcefile_size;
    unsigned int packedfile_size;
};

typedef struct fileinfo_s fileinfo;

struct filelist_s {
    char *root;
    fileinfo *files;
    unsigned int total;
};

typedef struct filelist_s filelist;

#ifdef __cplusplus
extern "C" {
#endif

filelist *filelist_new(void);
void filelist_free(filelist *flist);
char *filelist_root(filelist *flist, const char *root);

fileinfo *filelist_find(filelist *flist, const char *filename);
fileinfo *filelist_append(filelist *flist, const char *filename, unsigned int samplerate);
fileinfo *filelist_update(filelist *flist, const char *filename, const char *fullpath);

int fileinfo_folder(fileinfo *info, const char *folder);
void filelist_totals(filelist *flist, uint64_t *total, uint64_t *source, uint64_t *packed, uint64_t *source_size, uint64_t *packed_size);


#ifdef _cplusplus
}
#endif

#endif
