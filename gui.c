#include "gui.h"
#if defined(_WIN32) || defined(_WIN64)
#ifndef _UNICODE
#define _UNICODE 1
#endif
#ifndef UNICODE
#define UNICODE 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>
#include <iup.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "audparse.h"
#include "probe.h"
#include "filelist.h"
#include "pack.h"
#include "unpack.h"
#include "projectlist.h"
#include "util.h"

static projectlist *plist;
static aud_parser *parser;
static filelist *flist;
static int hide_warning;

static Ihandle *dlg;
static Ihandle *instLabel;
static Ihandle *statusLabel;
static Ihandle *packButton, *unpackButton;
static Ihandle *openButton, *fileText;
static Ihandle *batchDlgBtn;
static Ihandle *progressBar;
static Ihandle *gridBox;
static Ihandle *vBox;

static Ihandle *batchDlg;
static Ihandle *batchInstLabel;
static Ihandle *batchStatusLabel;
static Ihandle *batchPackButton, *batchUnpackButton;
static Ihandle *batchOpenButton, *batchFileText;
static Ihandle *batchProgressBar;
static Ihandle *batchGridBox;
static Ihandle *batchVBox;

static char *size_fmt;

static char *status;
static char *status2;
static char *act_status;
static char *inact_status;

static char *batch_status;
static char *batch_status2;
static char *batch_act_status;
static char *batch_inact_status;

static void clean(void) {
    if(parser != NULL) aud_free(parser);
    if(flist != NULL) filelist_free(flist);
    if(status != NULL) free(status);
    if(status2 != NULL) free(status2);
    if(size_fmt != NULL) free(size_fmt);
    if(plist != NULL) projectlist_free(plist);
    if(batch_status != NULL) free(batch_status);
    if(batch_status2 != NULL) free(batch_status2);
}

static void swapBatchStatus(void) {
    char *status_temp;
    status_temp = batch_act_status;
    batch_act_status = batch_inact_status;
    batch_inact_status = status_temp;
    IupSetAttribute(batchStatusLabel,"TITLE",batch_act_status);
}

static void swapStatus(void) {
    char *status_temp;
    status_temp = act_status;
    act_status = inact_status;
    inact_status = status_temp;
    IupSetAttribute(statusLabel,"TITLE",act_status);
}

static int unpackFiles(void) {
    unsigned int i = 0;
    unsigned int status_len;
    for(i=0;i<flist->total;i++) {
        status_len = snprintf(NULL,0,"Status: unpacking %u/%u",i+1,flist->total);
        inact_status = realloc(inact_status,status_len);
        snprintf(inact_status,status_len+1,"Status: unpacking %u/%u",i+1,flist->total);

        swapStatus();
        IupSetfAttribute(progressBar,"VALUE","%g", (double) i / (double)flist->total);
        if(IupLoopStep() == IUP_CLOSE) {
            IupExitLoop();
            return IUP_CLOSE;
        }
        if(unpack(flist,i) != 0) {
            IupMessage("Error","error unpacking");
            return IUP_CLOSE;
        }
    }
    IupSetfAttribute(progressBar,"VALUE","%g", 1.0f);
    return IUP_DEFAULT;
}

static int packFiles(void) {
    unsigned int i = 0;
    unsigned int status_len;
    for(i=0;i<flist->total;i++) {
        status_len = snprintf(NULL,0,"Status: packing %u/%u",i+1,flist->total);
        inact_status = realloc(inact_status,status_len);
        snprintf(inact_status,status_len+1,"Status: packing %u/%u",i+1,flist->total);

        swapStatus();
        IupSetfAttribute(progressBar,"VALUE","%g", (double) i / (double)flist->total);
        if(IupLoopStep() == IUP_CLOSE) {
            IupExitLoop();
            return IUP_CLOSE;
        }
        if(pack(flist,i) != 0) {
            IupMessage("Error","error unpacking");
            return IUP_CLOSE;
        }
    }
    IupSetfAttribute(progressBar,"VALUE","%g", 1.0f);
    return IUP_DEFAULT;
}

static void loadProjects(const char *folder) {
    unsigned int status_len;
    if(plist != NULL) projectlist_free(plist);
    plist = projectlist_new();
    projectlist_fill(plist,folder);
    status_len = snprintf(NULL,0,"Status: found %u projects",plist->total);
    batch_inact_status = realloc(batch_inact_status,status_len+1);
    status_len = snprintf(batch_inact_status,status_len+1,"Status: found %u projects",plist->total);

    if(plist->total > 0) {
        IupSetAttribute(batchFileText,"VALUE",folder);
        IupSetAttribute(batchUnpackButton,"ACTIVE","YES");
        IupSetAttribute(batchPackButton,"ACTIVE","YES");
    }

    swapBatchStatus();

    return;
}

static int loadAudacity(const char *file) {
    unsigned int status_len;
    uint64_t total;
    uint64_t source;
    uint64_t packed;
    uint64_t source_size;
    uint64_t packed_size;
    int aud_res;
    unsigned int chunk_pos = 0;
    unsigned int chunk_size = 0;
    double progress = 0.0f;
    int pret;
    if(parser == NULL) {
        parser = aud_new();
        if(parser == NULL) {
            IupMessage("Error","out of memory");
            return IUP_CLOSE;
        }
    }
    if(aud_load(parser,file) != 0) {
        IupMessage("Error","out of memory");
        return IUP_CLOSE;
    }

    if(flist != NULL) filelist_free(flist);
    flist = filelist_new();

    IupSetfAttribute(progressBar,"VALUE","%g", 0.0f);
    if(IupLoopStep() == IUP_CLOSE) {
        IupExitLoop();
        return IUP_CLOSE;
    }
    chunk_size = parser->xmlsize / 10;
    do {
        aud_res = aud_parse(parser,flist);
        chunk_pos++;
        if(chunk_pos == chunk_size) {
            progress += 0.10f;
            IupSetfAttribute(progressBar,"VALUE","%g", progress);
            chunk_pos = 0;
            if(IupLoopStep() == IUP_CLOSE) {
                IupExitLoop();
                return IUP_CLOSE;
            }
        }
    } while(aud_res == 0);

    switch(aud_res) {
        case -1: {
            filelist_free(flist);
            flist =  NULL;
            IupMessage("Error","Failed to load Audacity project");
            return IUP_CLOSE;
        }
        case -2: {
            if(hide_warning == 0) {
                IupMessage("Warning","Found aliased audio.\nUse File -> Save Lossless Copy of Project\nin Audacity");
            }
            break;
        }
    }
   IupSetfAttribute(progressBar,"VALUE","%g", 1.0f);

   pret = probe(file,flist);
   if(pret == -1) {
       IupMessage("Error","out of memory");
       return IUP_CLOSE;
   }

   IupSetAttribute(fileText,"VALUE",file);
   filelist_totals(flist,&total,&source,&packed,&source_size,&packed_size);
   if(source > packed) {
       size_fmt = util_hsize(source_size);
       status_len = snprintf(NULL,0,"Status: loaded, %" PRIu64 " raw files, %s",source,size_fmt);
       inact_status = realloc(inact_status,status_len+1);
       status_len = snprintf(inact_status,status_len+1,"Status: loaded, %" PRIu64 " raw files, %s",source,size_fmt);
       IupSetAttribute(packButton,"ACTIVE","YES");
       IupSetAttribute(unpackButton,"ACTIVE","NO");
   }
   else {
       size_fmt = util_hsize(packed_size);
       status_len = snprintf(NULL,0,"Status: loaded, %" PRIu64 " packed files, %s",packed,size_fmt);
       inact_status = realloc(inact_status,status_len+1);
       status_len = snprintf(inact_status,status_len+1,"Status: loaded, %" PRIu64 " packed files, %s",packed,size_fmt);
       IupSetAttribute(unpackButton,"ACTIVE","YES");
       IupSetAttribute(packButton,"ACTIVE","NO");
   }
   swapStatus();
   fprintf(stderr,"Setting status to %s\n",act_status);
   free(size_fmt);
   size_fmt = NULL;
   return IUP_DEFAULT;
}

static int batchOpenBtnCb(Ihandle *self) {
    (void)self;
    char *folder;
    Ihandle *fileDlg = IupFileDlg();
    IupSetAttribute(fileDlg,"DIALOGTYPE","DIR");
    IupSetAttribute(fileDlg,"TITLE","Choose a folder");
    IupPopup(fileDlg,IUP_CURRENT,IUP_CURRENT);
    if(IupGetInt(fileDlg,"STATUS") == 0) {
        folder = IupGetAttribute(fileDlg,"VALUE");
        loadProjects(folder);
    }
    return IUP_DEFAULT;
}

static int openBtnCb(Ihandle *self) {
    (void)self;
    char *file;
    Ihandle *fileDlg = IupFileDlg();
    IupSetAttribute(fileDlg,"DIALOGTYPE","OPEN");
    IupSetAttribute(fileDlg,"TITLE","Choose an Audacity project");
    IupSetAttribute(fileDlg,"FILTER","*.aup");
    IupSetAttribute(fileDlg,"FILTERINFO","Audacity Project");
    IupPopup(fileDlg,IUP_CURRENT,IUP_CURRENT);
    if(IupGetInt(fileDlg,"STATUS") == 0) {
        file = IupGetAttribute(fileDlg,"VALUE");
        loadAudacity(file);
    }
    return IUP_DEFAULT;
}

static int batchPackBtnCb(Ihandle *self) {
    (void)self;
    unsigned int i = 0;
    unsigned int status_len;
    for(i=0;i<plist->total;i++) {
       status_len = snprintf(NULL,0,"Status: batch packing %u/%u",i+1,plist->total);
       batch_inact_status = realloc(batch_inact_status,status_len);
       status_len = snprintf(batch_inact_status,status_len+1,"Status: batch packing %u/%u",i+1,plist->total);
       swapBatchStatus();
       IupSetfAttribute(batchProgressBar,"VALUE","%g", (double) i / (double)plist->total);
       loadAudacity(plist->projects[i]);
       if(packFiles() != IUP_DEFAULT) {
           return IUP_CLOSE;
       }
       hide_warning = 1;
       loadAudacity(plist->projects[i]);
       hide_warning = 0;
    }
    IupSetfAttribute(batchProgressBar,"VALUE","%g", 1.0f);
    return IUP_DEFAULT;
}

static int batchUnpackBtnCb(Ihandle *self) {
    (void)self;
    unsigned int i = 0;
    unsigned int status_len;
    for(i=0;i<plist->total;i++) {
       status_len = snprintf(NULL,0,"Status: batch unpacking %u/%u",i+1,plist->total);
       batch_inact_status = realloc(batch_inact_status,status_len);
       status_len = snprintf(batch_inact_status,status_len+1,"Status: unbatch packing %u/%u",i+1,plist->total);
       swapBatchStatus();
       IupSetfAttribute(batchProgressBar,"VALUE","%g", (double) i / (double)plist->total);
       loadAudacity(plist->projects[i]);
       if(unpackFiles() != IUP_DEFAULT) {
           return IUP_CLOSE;
       }
       hide_warning = 1;
       loadAudacity(plist->projects[i]);
       hide_warning = 0;
    }
    IupSetfAttribute(batchProgressBar,"VALUE","%g", 1.0f);
    return IUP_DEFAULT;
}


static int packBtnCb(Ihandle *self) {
    (void)self;
    char *file = IupGetAttribute(fileText,"VALUE");
    if(packFiles() != IUP_DEFAULT) {
        return IUP_CLOSE;
    }
    loadAudacity(file);
    return IUP_DEFAULT;
}

static int unpackBtnCb(Ihandle *self) {
    (void)self;
    char *file = IupGetAttribute(fileText,"VALUE");
    if(unpackFiles() != IUP_DEFAULT) {
        return IUP_CLOSE;
    }
    loadAudacity(file);
    return IUP_DEFAULT;
}

static int batchDlgBtnCb(Ihandle *self) {
    (void)self;
    batchInstLabel = IupLabel("Choose a folder of Audacity projects");
    batchStatusLabel = IupLabel(NULL);
    IupSetAttribute(batchStatusLabel,"EXPAND","HORIZONTAL");

    batchOpenButton = IupButton("Open", NULL);
    IupSetCallback(batchOpenButton,"ACTION", (Icallback) batchOpenBtnCb);

    batchFileText = IupText(NULL);
    IupSetAttribute(batchFileText,"EXPAND","HORIZONTAL");

    batchPackButton = IupButton("Pack",NULL);
    IupSetAttribute(batchPackButton,"ACTIVE","NO");
    IupSetCallback(batchPackButton,"ACTION", (Icallback) batchPackBtnCb);

    batchUnpackButton = IupButton("Unpack",NULL);
    IupSetAttribute(batchUnpackButton,"ACTIVE","NO");
    IupSetCallback(batchUnpackButton,"ACTION", (Icallback) batchUnpackBtnCb);

    batch_status = realloc(batch_status,strlen("Status: ready")+1);
    strcpy(batch_status,"Status: ready");
    IupSetAttribute(batchStatusLabel,"TITLE",batch_status);

    batchProgressBar = IupProgressBar();
    IupSetAttribute(batchProgressBar,"EXPAND","HORIZONTAL");

    batchGridBox = IupGridBox(batchOpenButton,batchFileText,batchPackButton,batchUnpackButton,NULL);
    IupSetAttribute(batchGridBox,"ORIENTATION","HORIZONTAL");
    IupSetAttribute(batchGridBox,"NUMDIV","2");
    IupSetAttribute(batchGridBox,"GAPLIN","20");
    IupSetAttribute(batchGridBox,"GAPCOL","20");
    IupSetAttribute(batchGridBox,"NORMALIZESIZE","HORIZONTAL");
    IupSetAttribute(batchGridBox,"TABTITLE","Basics");

    batchVBox = IupVbox(batchInstLabel,batchGridBox,batchStatusLabel,batchProgressBar,NULL);
    IupSetAttribute(batchVBox,"NMARGIN","10x10");
    IupSetAttribute(batchVBox,"NGAP","20");

    batchDlg = IupDialog(batchVBox);
    IupSetAttribute(batchDlg,"TITLE","Batch Mode");
    IupSetAttribute(batchDlg,"SIZE","300x170");
    IupShowXY(batchDlg,IUP_MOUSEPOS,IUP_MOUSEPOS);

    return IUP_DEFAULT;
}


int start_gui(int argc, char *argv[]) {
    flist = NULL;
    plist = NULL;
    parser = NULL;
    status = NULL;
    status2 = NULL;
    batch_status = NULL;
    batch_status2 = NULL;
    size_fmt = NULL;
    batchDlg = NULL;

    IupOpen(&argc, &argv);
    IupSetGlobal("UTF8MODE","YES");
    IupSetGlobal("UTF8MODE_FILE","YES");

    instLabel = IupLabel("Choose an Audacity project");
    statusLabel = IupLabel(NULL);
    IupSetAttribute(statusLabel,"EXPAND","HORIZONTAL");

    openButton = IupButton("Open", NULL);
    IupSetCallback(openButton,"ACTION", (Icallback) openBtnCb);

    fileText = IupText(NULL);
    IupSetAttribute(fileText,"EXPAND","HORIZONTAL");

    packButton = IupButton("Pack",NULL);
    IupSetAttribute(packButton,"ACTIVE","NO");
    IupSetCallback(packButton,"ACTION", (Icallback) packBtnCb);

    unpackButton = IupButton("Unpack",NULL);
    IupSetAttribute(unpackButton,"ACTIVE","NO");
    IupSetCallback(unpackButton,"ACTION", (Icallback) unpackBtnCb);

    batchDlgBtn = IupButton("Batch Mode",NULL);
    IupSetCallback(batchDlgBtn,"ACTION",(Icallback)batchDlgBtnCb);

    status = realloc(status,strlen("Status: ready")+1);
    strcpy(status,"Status: ready");
    IupSetAttribute(statusLabel,"TITLE",status);

    act_status = status;
    inact_status = status2;

    batch_act_status = batch_status;
    batch_inact_status = batch_status2;
    hide_warning = 0;

    progressBar = IupProgressBar();
    IupSetAttribute(progressBar,"EXPAND","HORIZONTAL");

    gridBox = IupGridBox(openButton,fileText,packButton,unpackButton,NULL);
    IupSetAttribute(gridBox,"ORIENTATION","HORIZONTAL");
    IupSetAttribute(gridBox,"NUMDIV","2");
    IupSetAttribute(gridBox,"GAPLIN","20");
    IupSetAttribute(gridBox,"GAPCOL","20");
    IupSetAttribute(gridBox,"NORMALIZESIZE","HORIZONTAL");
    IupSetAttribute(gridBox,"TABTITLE","Basics");

    vBox = IupVbox(instLabel,gridBox,statusLabel,progressBar,batchDlgBtn,NULL);
    IupSetAttribute(vBox,"NMARGIN","10x10");
    IupSetAttribute(vBox,"NGAP","20");

    dlg = IupDialog(vBox);
    IupSetAttribute(dlg,"TITLE","Audacity Packer");
    IupSetAttribute(dlg,"SIZE","300x170");
    IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
    IupMainLoop();
    IupClose();
    clean();

    return 0;
}

#else

int start_gui(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    return -1;
}

#endif
