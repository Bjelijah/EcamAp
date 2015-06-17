//
//  aac_adtstoasc_filter.c
//  GLCameraRipple
//
//  Created by howell on 14-6-30.
//
//

#include <stdio.h>
#include "libavcodec/avcodec.h"
#include "aac_adtstoasc_filter.h"
#include "aacadtsdec.h"
#include "get_bits.h"
#include "put_bits.h"
#include "mpeg4audio.h"

#define INT_MAX 0x7fffffff
#define AVERROR_INVALIDDATA -1

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;




/**
 * This filter creates an MPEG-4 AudioSpecificConfig from an MPEG-2/4
 * ADTS header and removes the ADTS header.
 */
int aac_adtstoasc_filter(AACBSFContext *ctx,
                                AVCodecContext *avctx, const char *args,
                                uint8_t  **poutbuf, int *poutbuf_size,
                                const uint8_t *buf, int      buf_size,
                                int keyframe)
{
    GetBitContext gb;
    PutBitContext pb;
    AACADTSHeaderInfo hdr;
        
    init_get_bits(&gb, buf, AAC_ADTS_HEADER_SIZE*8);
    
    *poutbuf = (uint8_t*) buf;
    *poutbuf_size = buf_size;
    
    if (avctx->extradata)
        if (show_bits(&gb, 12) != 0xfff)
            return 0;
    
    if (avpriv_aac_parse_header(&gb, &hdr) < 0) {
        printf("Error parsing ADTS frame header!\n");
        return -1;
    }
    
    if (!hdr.crc_absent && hdr.num_aac_frames > 1) {
        printf("Multiple RDBs per frame with CRC\n");
        return AVERROR_PATCHWELCOME;
    }
    
    buf      += AAC_ADTS_HEADER_SIZE + 2*!hdr.crc_absent;
    buf_size -= AAC_ADTS_HEADER_SIZE + 2*!hdr.crc_absent;
    
    if (!ctx->first_frame_done) {
        int            pce_size = 0;
        uint8_t        pce_data[MAX_PCE_SIZE];
        if (!hdr.chan_config) {
            init_get_bits(&gb, buf, buf_size * 8);
            if (get_bits(&gb, 3) != 5) {
                av_log_missing_feature(avctx, "PCE based channel configuration, where the PCE is not the first syntax element", 0);
                return AVERROR_PATCHWELCOME;
            }
            init_put_bits(&pb, pce_data, MAX_PCE_SIZE);
            pce_size = avpriv_copy_pce_data(&pb, &gb)/8;
            flush_put_bits(&pb);
            buf_size -= get_bits_count(&gb)/8;
            buf      += get_bits_count(&gb)/8;
        }
        avctx->extradata_size = 2 + pce_size;
        avctx->extradata = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
        
        init_put_bits(&pb, avctx->extradata, avctx->extradata_size);
        put_bits(&pb, 5, hdr.object_type);
        put_bits(&pb, 4, hdr.sampling_index);
        put_bits(&pb, 4, hdr.chan_config);
        put_bits(&pb, 1, 0); //frame length - 1024 samples
        put_bits(&pb, 1, 0); //does not depend on core coder
        put_bits(&pb, 1, 0); //is not extension
        flush_put_bits(&pb);
        if (pce_size) {
            memcpy(avctx->extradata + 2, pce_data, pce_size);
        }
        
        ctx->first_frame_done = 1;
    }
    
    *poutbuf = (uint8_t*) buf;
    *poutbuf_size = buf_size;
    
    return 0;
}