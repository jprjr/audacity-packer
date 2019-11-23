#include "cli.h"

#define USAGE "Usage: %s [--batch pack|unpack] /path/to/project.aup|folder ...\n"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "probe.h"
#include "filelist.h"
#include "pack.h"
#include "unpack.h"
#include "projectlist.h"
#include "util.h"

static void projectlist_dump(projectlist *plist) {
    unsigned int i = 0;
    for(i=0;i<plist->total;i++) {
        fprintf(stderr,"Project: %s\n",plist->projects[i]);
    }
}

int start_cli(int argc, char *argv[]) {
    unsigned int i = 0;
    unsigned int j = 0;
    int batch = 0;
    char buffer[100];
    projectlist *plist;
    filelist *flist;

    if(argc < 2) {
        fprintf(stderr,USAGE,argv[0]);
        return 1;
    }

    plist = projectlist_new();
    if(plist == NULL) return 1;
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
        if(batch == 0) {
            fprintf(stdout,"%s - pack/unpack/skip: ", plist->projects[i]);
            fflush(stdout);
            fgets(buffer,99,stdin);
            if(strncmp(buffer,"skip",strlen("skip")) == 0) {
                fprintf(stdout,"Skipping\n");
                continue;
            }
            if(flist != NULL) {
                filelist_free(flist);
                flist = NULL;
            }
            flist = filelist_new();
            if(flist == NULL) return 1;

            if(strncmp(buffer,"pack",strlen("pack")) == 0) {
                probe(plist->projects[i],flist);
                for(j=0;j<flist->total;j++) {
                    fprintf(stdout,"[%s]: packing %u/%u\n",plist->projects[i],j+1,flist->total);
                    pack(flist,j);
                }
            }
            else if(strncmp(buffer,"unpack",strlen("unpack")) == 0) {
                probe(plist->projects[i],flist);
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
            if(flist != NULL) {
                filelist_free(flist);
                flist = NULL;
            }
            flist = filelist_new();
            if(flist == NULL) return 1;
            probe(plist->projects[i],flist);
            for(j=0;j<flist->total;j++) {
                fprintf(stdout,"[%s]: packing %u/%u\n",plist->projects[i],j+1,flist->total);
                pack(flist,j);
            }
        }
        else if(batch == 2) {
            if(flist != NULL) {
                filelist_free(flist);
                flist = NULL;
            }
            flist = filelist_new();
            if(flist == NULL) return 1;
            probe(plist->projects[i],flist);
            for(j=0;j<flist->total;j++) {
                fprintf(stdout,"[%s]: unpacking %u/%u\n",plist->projects[i],j+1,flist->total);
                unpack(flist,j);
            }
        }
    }

    projectlist_dump(plist);
    projectlist_free(plist);
    if(flist != NULL) filelist_free(flist);
    return 0;
}

