//
//  adts_analyze.c
//  GLCameraRipple
//
//  Created by howell on 14-7-10.
//
//

#include <stdio.h>

static unsigned const aac_sample_idx[16] = {
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350, 0, 0, 0
};



void get_audio_info_from_adts(unsigned char *adtsBuf , int len, int * samplerate,int *channels)
{
    int sampleIndex = (adtsBuf[2] & 0x3c) >> 2;
    *samplerate = aac_sample_idx[sampleIndex];
    *channels =  ((adtsBuf[2] & 0x01) << 2) | ((adtsBuf[3] & 0xc0) >> 6);
}