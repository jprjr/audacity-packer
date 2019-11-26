#include "cli.h"

#define USAGE "Usage: %s [--batch pack|unpack] /path/to/project.aup|folder ...\n"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "audparse.h"
#include "probe.h"
#include "filelist.h"
#include "pack.h"
#include "unpack.h"
#include "projectlist.h"
#include "util.h"

int start_cli(int argc, char *argv[]) {
    int aud_res;
    size_t chunk_pos = 0;
    size_t chunk_tot = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    int batch = 0;
    char buffer[100];
    char *source_size_fmt;
    char *packed_size_fmt;
    aud_parser *parser;
    projectlist *plist;
    filelist *flist;

    uint64_t total_files;
    uint64_t source_files;
    uint64_t packed_files;
    uint64_t source_size;
    uint64_t packed_size;

    if(argc < 2) {
        fprintf(stderr,USAGE,argv[0]);
        return 1;
    }

    parser = aud_new();
    if(parser == NULL) return 1;
    plist = projectlist_new();
    if(plist == NULL) {
        aud_free(parser);
        return 1;
    }
    flist = NULL;

    for(i=1;i<(unsigned int)argc;i++) {
        if(strcmp(argv[i],"-b") == 0) {
            i++;
            if(strcmp(argv[i],"pack") == 0) {
                batch = 1;
            } else if(strcmp(argv[i],"unpack") == 0) {
                batch = 2;
            } else {
                fprintf(stderr,USAGE,argv[0]);
                return 1;
            }
        } else if(strcmp(argv[i],"--batch") == 0) {
            i++;
            if(strcmp(argv[i],"pack") == 0) {
                batch = 1;
            } else if(strcmp(argv[i],"unpack") == 0) {
                batch = 2;
            } else {
                fprintf(stderr,USAGE,argv[0]);
                return 1;
            }
        }
        else {
            if(util_isdir(argv[i])) {
                projectlist_fill(plist,argv[i]);
            }
            else if(util_isfile(argv[i])) {
                projectlist_append(plist,argv[i]);
            }
        }
    }

    for(i=0;i<plist->total;i++) {
        if(flist != NULL) {
            filelist_free(flist);
            flist = NULL;
        }

        fprintf(stdout,"Loading %s\n",plist->projects[i]);
        fflush(stdout);

        if(aud_load(parser,plist->projects[i]) != 0) {
            fprintf(stdout,"out of memory\n");
            aud_free(parser);
            projectlist_free(plist);
            return 1;
        }
        flist = filelist_new();
        if(flist == NULL) {
            fprintf(stdout,"out of memory\n");
            aud_free(parser);
            projectlist_free(plist);
            return 1;
        }
        chunk_pos = 0;
        chunk_tot = parser->xmlsize / 10;
        do {
            aud_res = aud_parse(parser,flist);
            chunk_pos++;
            if(chunk_pos == chunk_tot) {
                fprintf(stdout,".");
                fflush(stdout);
                chunk_pos = 0;
            }
        } while(aud_res == 0);

        switch(aud_res) {
            case -2: fprintf(stderr,"warning: found alias block files\n"); break;
            case -1: {
                fprintf(stderr,"unrecoverable error\n");
                aud_free(parser);
                projectlist_free(plist);
                filelist_free(flist);
            }
            return -1;
            default: fprintf(stdout," complete\n"); fflush(stdout);
        }
        aud_unload(parser);
        probe(plist->projects[i],flist);
        filelist_totals(flist,&total_files,&source_files,&packed_files,&source_size,&packed_size);
        source_size_fmt = util_hsize(source_size);
        packed_size_fmt = util_hsize(packed_size);
        fprintf(stdout,"Project %s - %" PRIu64 " source files (%s), %" PRIu64 " packed files (%s)\n",
          plist->projects[i],
          source_files,
          source_size_fmt,
          packed_files,
          packed_size_fmt);
        free(source_size_fmt);
        free(packed_size_fmt);

        if(batch == 0) {
            fprintf(stdout,"pack/unpack/skip: ");
            fflush(stdout);
            fgets(buffer,99,stdin);
            if(strncmp(buffer,"skip",strlen("skip")) == 0) {
                fprintf(stdout,"Skipping\n");
                continue;
            }

            if(strncmp(buffer,"pack",strlen("pack")) == 0) {
                for(j=0;j<flist->total;j++) {
                    fprintf(stdout,"[%s]: packing %u/%u\n",plist->projects[i],j+1,flist->total);
                    pack(flist,j);
                }
            }
            else if(strncmp(buffer,"unpack",strlen("unpack")) == 0) {
                for(j=0;j<flist->total;j++) {
                    fprintf(stdout,"[%s]: unpacking %u/%u\n",plist->projects[i],j+1,flist->total);
                    unpack(flist,j);
                }
            }
            else {
                fprintf(stdout,"Unknown command, quitting\n");
                return 1;
            }
        }
        else if(batch == 1) {
            for(j=0;j<flist->total;j++) {
                fprintf(stdout,"[%s]: packing %u/%u\n",plist->projects[i],j+1,flist->total);
                pack(flist,j);
            }
        }
        else if(batch == 2) {
            for(j=0;j<flist->total;j++) {
                fprintf(stdout,"[%s]: unpacking %u/%u\n",plist->projects[i],j+1,flist->total);
                unpack(flist,j);
            }
        }
    }

    aud_free(parser);
    projectlist_free(plist);
    if(flist != NULL) filelist_free(flist);

    return 0;
}

