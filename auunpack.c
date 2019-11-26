#if defined(__linux__)
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define FSEEK(a,b,c) _fseeki64(a,b,c)
#define FTELL(a) _ftelli64(a)
#else
#define FSEEK(a,b,c) fseeko(a,b,c)
#define FTELL(a) ftello(a)
#endif

#include "fopen_wrapper.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wavpack/wavpack.h>

static uint8_t pack_le_int16(uint8_t *d, int32_t s) {
    d[0] = (uint8_t) (s      );
    d[1] = (uint8_t) (s >> 8 );
    return 2;
}


static uint8_t pack_be_int16(uint8_t *d, int32_t s) {
    d[0] = (uint8_t) (s >> 8 );
    d[1] = (uint8_t) (s      );
    return 2;
}

static uint8_t pack_le_int24(uint8_t *d, int32_t s) {
    d[0] = (uint8_t) (s       );
    d[1] = (uint8_t) (s >> 8  );
    d[2] = (uint8_t) (s >> 16 );
    return 3;
}

static uint8_t pack_be_int24(uint8_t *d, int32_t s) {
    d[0] = (uint8_t) (s >> 16 );
    d[1] = (uint8_t) (s >> 8  );
    d[2] = (uint8_t) (s       );
    return 3;
}

static uint8_t pack_le_int32(uint8_t *d, int32_t s) {
    d[0] = (uint8_t) (s       );
    d[1] = (uint8_t) (s >> 8  );
    d[2] = (uint8_t) (s >> 16 );
    d[3] = (uint8_t) (s >> 24 );
    return 4;
}

static uint8_t pack_be_int32(uint8_t *d, int32_t s) {
    d[0] = (uint8_t) (s >> 24 );
    d[1] = (uint8_t) (s >> 16 );
    d[2] = (uint8_t) (s >> 8  );
    d[3] = (uint8_t) (s       );
    return 4;
}

static int32_t wavpack_read(void *id, void *data, int32_t bcount) {
    int32_t r = (int32_t)fread(data,1,bcount,(FILE *)id);
    return r;
}

static uint32_t wavpack_pos(void *id) {
    uint32_t r = (uint32_t)FTELL((FILE *)id);
    return r;
}

static int wavpack_set_pos_abs(void *id, uint32_t pos) {
    int set_pos = FSEEK((FILE *)id,(long int)pos,SEEK_SET);
    return set_pos;
}

static int wavpack_set_pos_rel(void *id, int32_t delta, int mode) {
    int set_pos = FSEEK((FILE *)id,(long int)delta,mode);
    return set_pos;
}

static int wavpack_push_back_byte(void *id, int c) {
    return ungetc(c,(FILE *)id);
}

static int wavpack_can_seek(void *id) {
    (void)id;
    return 1;
}

static uint32_t wavpack_get_length(void *id) {
    FILE *f = (FILE *)id;
    uint32_t len;
    uint32_t pos;
    pos = (uint32_t)FTELL(f);
    FSEEK(f,0,SEEK_END);
    len = (uint32_t)FTELL(f);
    FSEEK(f,pos,SEEK_SET);
    return len;
}


unsigned int auunpack(const char *input, const char *output) {
    FILE *infile;
    FILE *outfile;
    WavpackStreamReader reader;
    WavpackContext *wp;
    unsigned int ret;
    int32_t *samples;
    uint32_t total_samples;
    int sample_size;
    uint8_t *wrapper;
    uint32_t wrapper_size;
    uint8_t (*packer)(uint8_t *, int32_t);
    uint8_t endian;
    uint8_t *packed;
    uint32_t i;
    int flags;

    infile = NULL;
    outfile = NULL;
    wp = NULL;
    samples = NULL;
    packed = NULL;
    ret = 0;
    endian = 0;
    flags = 0;

    DEBUG_LOG("Unpacking %s to %s\n",input,output);

    infile = fopen_wrapper(input,"rb");
    if(infile == NULL) goto error;

    reader.read_bytes = wavpack_read;
    reader.get_pos = wavpack_pos;
    reader.set_pos_abs = wavpack_set_pos_abs;
    reader.set_pos_rel = wavpack_set_pos_rel;
    reader.push_back_byte = wavpack_push_back_byte;
    reader.get_length = wavpack_get_length;
    reader.can_seek = wavpack_can_seek;
    reader.write_bytes = NULL;

    flags |= OPEN_WRAPPER;

    wp = WavpackOpenFileInputEx(&reader,
      infile,
      NULL,
      NULL,
      flags,
      0);
    if(wp == NULL) goto error;

    total_samples = WavpackGetNumSamples(wp);
    sample_size = WavpackGetBytesPerSample(wp);
    samples = (int32_t *)malloc(sizeof(int32_t) * total_samples);
    if(samples == NULL) goto error;
    packed = (uint8_t *)malloc(sizeof(uint8_t) * sample_size);
    if(packed == NULL) goto error;

    outfile = fopen_wrapper(output,"wb");
    if(outfile == NULL) goto error;

    wrapper_size = WavpackGetWrapperBytes(wp);
    if(wrapper_size == 0) {
        goto error;
    }

    wrapper = WavpackGetWrapperData(wp);
    if(wrapper == NULL) {
        goto error;
    }

    if(memcmp(wrapper,".snd",4) == 0) {
        endian = 1;
    }

    if(fwrite(wrapper,1,wrapper_size,outfile) != wrapper_size) goto error;
    WavpackFreeWrapper(wp);

    if(WavpackUnpackSamples(wp,samples,total_samples) != total_samples) goto error;
    switch(sample_size) {
        case 2: packer = (endian == 0 ? pack_le_int16 : pack_be_int16); break;
        case 3: packer = (endian == 0 ? pack_le_int24 : pack_be_int24); break;
        case 4: packer = (endian == 0 ? pack_le_int32 : pack_be_int32); break;
        default: goto error;
    }

    for(i=0;i<total_samples;i++) {
        packer(packed,samples[i]);
        if((int)fwrite(packed,1,sample_size,outfile) != sample_size) goto error;
    }

    DEBUG_LOG("Unpack successful\n");
    goto cleanup;

error:
    ret = 1;
cleanup:
    if(wp != NULL) WavpackCloseFile(wp);
    if(infile != NULL) fclose(infile);
    if(outfile != NULL) fclose(outfile);
    if(samples != NULL) free(samples);
    if(packed != NULL) free(packed);

    return ret;
}
