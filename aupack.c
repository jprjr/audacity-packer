#include "fopen_wrapper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <wavpack/wavpack.h>

#if 0
static uint8_t unpack_le_int8(int32_t *d, uint8_t *s) {
    *d = (int8_t) *s;
    return 1;
}

static uint8_t unpack_le_uint8(int32_t *d, uint8_t *s) {
    *d = *s - 0x80;
    return 1;
}

static uint8_t unpack_be_int8(int32_t *d, uint8_t *s) {
  return unpack_le_int8(d,s);
}

static uint8_t unpack_be_uint8(int32_t *d, uint8_t *s) {
  return unpack_le_uint8(d,s);
}

static uint8_t unpack_le_uint16(int32_t *d, uint8_t *s) {
    *d = (s[0] | ((int32_t)s[1] << 8)) - 0x8000;
    return 2;
}

static uint8_t unpack_be_uint16(int32_t *d, uint8_t *s) {
    *d = (s[1] | ((int32_t)s[2] << 8)) - 0x8000;
    return 2;
}

static uint8_t unpack_le_uint24(int32_t *d, uint8_t *s) {
    *d = (s[0] | ((int32_t)s[1] << 8) | ((int32_t)(int8_t)s[2] << 16)) - 0x800000;
    return 3;
}

static uint8_t unpack_be_uint24(int32_t *d, uint8_t *s) {
    *d = (s[2] | ((int32_t)s[1] << 8) | ((int32_t)(int8_t)s[0] << 16)) - 0x800000;
    return 3;
}

static uint8_t unpack_le_uint32(int32_t *d, uint8_t *s) {
    *d = (s[0] | ((int32_t)s[1] << 8) | ((int32_t)s[2] << 16) | ((int32_t)(int8_t)s[3] << 24)) - 0x80000000;
    return 4;
}

static uint8_t unpack_be_uint32(int32_t *d, uint8_t *s) {
    *d = (s[3] | ((int32_t)s[2] << 8) | ((int32_t)s[1] << 16) | ((int32_t)(int8_t)s[0] << 24)) - 0x80000000;
    return 4;
}

#endif

static uint8_t unpack_le_int16(int32_t *d, uint8_t *s) {
    *d = (s[0] | ((int32_t)(int8_t)s[1] << 8));
    return 2;
}


static uint8_t unpack_be_int16(int32_t *d, uint8_t *s) {
    *d = (s[1] | ((int32_t)(int8_t)s[2] << 8));
    return 2;
}

static uint8_t unpack_le_int24(int32_t *d, uint8_t *s) {
    *d = (s[0] | ((int32_t)s[1] << 8) | ((int32_t)(int8_t)s[2] << 16));
    return 3;
}

static uint8_t unpack_be_int24(int32_t *d, uint8_t *s) {
    *d = (s[2] | ((int32_t)s[1] << 8) | ((int32_t)(int8_t)s[0] << 16));
    return 3;
}

static uint8_t unpack_le_int32(int32_t *d, uint8_t *s) {
    *d = (s[0] | ((int32_t)s[1] << 8) | ((int32_t)s[2] << 16) | ((int32_t)(int8_t)s[3] << 24));
    return 4;
}


static uint8_t unpack_be_int32(int32_t *d, uint8_t *s) {
    *d = (s[3] | ((int32_t)s[2] << 8) | ((int32_t)s[1] << 16) | ((int32_t)(int8_t)s[0] << 24));
    return 4;
}


static uint32_t read_word_le(const uint8_t *b) {
    return
           (b[0]       ) +
           (b[1] << 8  ) +
           (b[2] << 16 ) +
           (b[2] << 24 );
}

static uint32_t read_word_be(const uint8_t *b) {
    return
           (b[3]       ) +
           (b[2] << 8  ) +
           (b[1] << 16 ) +
           (b[0] << 24 );
}

static int wavpack_write(void *id, void *data, int32_t bcount) {
    return (int32_t)fwrite(data,1,bcount,(FILE *)id) == bcount;
}

unsigned int aupack(const char *input, const char *output, unsigned int samplerate) {
    (void)samplerate;
    FILE *infile;
    FILE *outfile;
    WavpackContext *wp;
    WavpackConfig wp_config;
    uint8_t endian = 0; /* 1 for big-endian */
    uint32_t offset = 0;
    uint32_t data_size = 0;
    unsigned int format = 0;
    unsigned int r_channels = 0;
    uint8_t buffer[2048] = {0};
    uint8_t *header;
    uint8_t *data;
    uint8_t *dp;
    int32_t *samples;
    int32_t *sp;
    uint8_t (*unpacker)(int32_t *, uint8_t *);
    unsigned int ret = 0;

    wp = NULL;
    infile = NULL;
    outfile = NULL;
    header = NULL;
    data = NULL;
    samples = NULL;

    memset(&wp_config,0,sizeof(WavpackConfig));

    infile = fopen_wrapper(input,"rb");
    if(infile == NULL) {
        fprintf(stderr,"error opening input\n");
        goto error;
    }
    if(fread(buffer,1,4,infile) != 4) {
        fprintf(stderr,"error reading magic\n");
        goto error;
    }

    if(memcmp(buffer,".snd",4)==0) {
        endian = 1;
    }
    else if(memcmp(buffer,"dns.",4) != 0) {
        fprintf(stderr,"error: not a snd file\n");
        goto error;
    }

    if(fread(buffer,1,4,infile) != 4) {
        fprintf(stderr,"error reading size\n");
        goto error;
    }

    offset = ( endian == 0 ? read_word_le(buffer) : read_word_be(buffer) );

    if(fread(buffer,1,16,infile) != 16) {
        fprintf(stderr,"error reading header\n");
        goto error;
    }
    format = ( endian == 0 ? read_word_le(&buffer[4]) : read_word_be(&buffer[4]));
    r_channels = ( endian == 0 ? read_word_le(&buffer[12]) : read_word_be(&buffer[12]));
    wp_config.flags |= CONFIG_VERY_HIGH_FLAG;

    switch(format) {
        case 3: {
            wp_config.bytes_per_sample = 2;
            wp_config.bits_per_sample = 16;
            unpacker = (endian == 0 ? unpack_le_int16 : unpack_be_int16);
            break;
        }
        case 4: {
            wp_config.bytes_per_sample = 3;
            wp_config.bits_per_sample = 24;
            unpacker = (endian == 0 ? unpack_le_int24 : unpack_be_int24);
            break;
        }
        case 6: {
            wp_config.bytes_per_sample = 4;
            wp_config.bits_per_sample = 32;
            unpacker = (endian == 0 ? unpack_le_int32 : unpack_be_int32);
            wp_config.float_norm_exp = 127;
            break;
        }
        default: {
            fprintf(stderr,"Unknown format id: %u\n",format);
            goto error;
        }
    }

    switch(r_channels) {
        case 1: {
            wp_config.num_channels = 1;
            wp_config.channel_mask = 4;
            break;
        }
        default: {
            fprintf(stderr,"Unsupported channel count: %u\n",r_channels);
            goto error;
        }
    }
    wp_config.sample_rate = samplerate;

    fseek(infile,0,SEEK_END);
    data_size = ftell(infile);
    data_size -= offset;

    fseek(infile,0,SEEK_SET);
    header = (uint8_t *)malloc(offset);
    if(header == NULL) {
        fprintf(stderr,"error mallocing header\n");
        goto error;
    }
    if(fread(header,1,offset,infile) != offset) {
        fprintf(stderr,"error reading entire header\n");
        goto error;
    }

    outfile = fopen_wrapper(output,"wb");
    if(outfile == NULL) {
        fprintf(stderr,"error opening output\n");
        goto error;
    }

    wp = WavpackOpenFileOutput(wavpack_write,outfile,NULL);
    if(wp == NULL) {
        fprintf(stderr,"error calling WavpackOpenFileOutput\n");
        goto error;
    }

    samples = (int32_t *)malloc(sizeof(int32_t) * (data_size / wp_config.bytes_per_sample));
    if(samples == NULL) {
        fprintf(stderr,"error mallocing samples\n");
        goto error;
    }
    data = (uint8_t *)malloc(data_size);
    if(data == NULL) {
        fprintf(stderr,"error mallocing data\n");
        goto error;
    }
    if(fread(data,1,data_size,infile) != data_size) {
        fprintf(stderr,"error reading data\n");
        goto error;
    }

    dp = data;
    sp = samples;

    WavpackAddWrapper(wp,header,offset);

    if(WavpackSetConfiguration(wp,&wp_config,data_size / wp_config.bytes_per_sample) == 0) {
        fprintf(stderr,"error in SetConfiguration: %s\n",WavpackGetErrorMessage(wp));
        goto error;
    }
    if(WavpackPackInit(wp) == 0) {
        fprintf(stderr,"error in PackInit: %s\n",WavpackGetErrorMessage(wp));
        goto error;
    }

    offset = 0;
    while(((size_t)(dp - data)) < data_size) {
        dp += unpacker(sp,dp);
        sp++;
    }

    if(WavpackPackSamples(wp,samples, data_size / wp_config.bytes_per_sample) == 0) {
        fprintf(stderr,"error in PackSamples: %s\n",WavpackGetErrorMessage(wp));
        goto error;
    }
    if(WavpackFlushSamples(wp) == 0) {
        fprintf(stderr,"error in FlushSamples: %s\n",WavpackGetErrorMessage(wp));
        goto error;
    }

    goto cleanup;
error:
    ret = 1;
cleanup:
    if(wp != NULL) WavpackCloseFile(wp);
    if(infile != NULL) fclose(infile);
    if(outfile != NULL) fclose(outfile);
    if(header != NULL) free(header);
    if(samples != NULL) free(samples);
    if(data != NULL) free(data);
    return ret;
}
