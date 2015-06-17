//
//  aac_adtstoasc_filter.h
//  GLCameraRipple
//
//  Created by howell on 14-6-30.
//
//

#ifndef GLCameraRipple_aac_adtstoasc_filter_h
#define GLCameraRipple_aac_adtstoasc_filter_h

typedef struct AACBSFContext {
    int first_frame_done;
} AACBSFContext;

int aac_adtstoasc_filter(AACBSFContext *ctx,
                                AVCodecContext *avctx, const char *args,
                                uint8_t  **poutbuf, int *poutbuf_size,
                                const uint8_t *buf, int      buf_size,
                                int keyframe);
#endif
