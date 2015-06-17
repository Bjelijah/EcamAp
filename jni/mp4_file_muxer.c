//
//  mp4_file_muxer.c
//  GLCameraRipple
//
//  Created by howell on 14-7-11.
//
//

#include "mp4_file_muxer.h"
#include <stdio.h>
#import "libavformat/avformat.h"
#import "libavcodec/avcodec.h"
#include "aac_adtstoasc_filter.h"
#include "h264_analyze.h"
#include "adts_analyze.h"
#include <android/log.h>


#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#define _M printf( "%s(%d) : MARKER\n", __FILE__, __LINE__ )
#define VIDEO_STREAM_INDEX 0
#define AUDIO_STREAM_INDEX 1
#define true  1
#define false 0

static short _hasCreateavcC;
static short _hasWriteHead;
static short _hasWriteOver;
static AVFormatContext *_formatContext;
static struct AACBSFContext _bsfc;
static int  waitkey = 1;
static int64_t _fristTimeStamp = 0;

static int spareSPSPPS(unsigned char* buf ,int len){
    int i;
    //AnnexB 0x67 + 0x68
    if ((buf[0]|buf[1]|buf[2] == 0x00) && (0x01 == buf[3]) && (buf[4] == 0x67)) {
        unsigned char * b;
        for (i = 0; i<len - 5; i++) {
            b = buf + i;
            if ((b[0]|b[1]|b[2] == 0x00) && (0x01 == b[3]) && (b[4] == 0x68)) {
                return i + 8; //返回SPS加PPS的长度
            }
        }
    }
    return 0;
}

// < 0 = error
// 0 = I-Frame
// 1 = P-Frame
// 2 = B-Frame
// 3 = S-Frame
static int getVopType( const void *p, int len )
{
    if ( !p || 6 >= len )
        return -1;
    
    unsigned char *b = (unsigned char*)p;
    int i = 0;
    for ( i; i < 5; i++) {
        printf("%02x ",b[i]);
    }
    printf("\n");
    /*
    // Verify NAL marker
    if ( b[0] || b[1] || 0x01 != b[2] ){
        b++;
        if ( b[0] || b[1] || 0x01 != b[2] )
            return -1;
    } // end if
    */
    
    /*MP4包首2个是0x00 0x00 */
    if (b[0] != 0 && b[1] != 0) {
        return -1;
    }
    
    b += 4;
    // Verify VOP id
    if ( 0xb6 == *b )
    {   b++;
        return ( *b & 0xc0 ) >> 6;
    } // end if
    
    printf("%02x \n",*b);
    switch( *b )
    {
        case 0x65 : return 0;
        case 0x61 : return 1;
        case 0x01 : return 2;
    } // end switch
    return -1;
}

static int get_nal_type( void *p, int len )
{
    if ( !p || 5 >= len )
        return -1;
    
    unsigned char *b = (unsigned char*)p;
    
    // Verify NAL marker
    if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
    {   b++;
        if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
            return -1;
    } // end if
    
    b += 3;
    
    return *b;
}

/**
 * 从h264的sps数据帧获取视评信息
 */

static int init_video_codecContext(char *buf ,int len)
{
    AVCodecContext *videoCodecContext = _formatContext->streams[VIDEO_STREAM_INDEX]->codec;
    int width;
    int height;
    int framerate;
    get_pic_info_fromSPS(buf+5,len-5, &width ,&height , &framerate);
    printf("height %d width %d ",height,width);
    videoCodecContext->width = width;
    videoCodecContext->height = height;

    
    av_dump_format(_formatContext, 0, _formatContext->filename, 1);
    
    if ( !( _formatContext->oformat->flags & AVFMT_NOFILE ) ) {
        int ret = avio_open( &_formatContext->pb, _formatContext->filename, AVIO_FLAG_WRITE );
        printf("avio_open %d %s",ret,_formatContext->filename);
    }
    int ret = avformat_write_header(_formatContext,NULL);
    if (ret != 0) {
        printf("init_video_codecContext:avformat_write_header error %d",ret);
    }
    return ret;
}

/**
 * 从aac的adts获取音频信息
 */

static void init_audio_codecContext(char* buf ,int len)
{
    AVCodecContext *audioCodecContext = _formatContext->streams[AUDIO_STREAM_INDEX]->codec;
    int sampleRate;
    int channels;
    get_audio_info_from_adts(buf,len,&sampleRate,&channels);
    printf("samplerate %d %d",sampleRate,channels);
    audioCodecContext->sample_rate = sampleRate;
    audioCodecContext->channels = channels;
}

/**
 * 解码第一个I帧获取,h264信息
 */

static void get_video_codec_context(unsigned char * buf,int len)
{
    
    avcodec_register_all();
    AVStream *stream = _formatContext->streams[VIDEO_STREAM_INDEX];
    AVCodec *codec = avcodec_find_decoder(CODEC_ID_H264);
    AVCodecContext * videoCodecContext = avcodec_alloc_context();
    videoCodecContext->flags |=CODEC_FLAG_EMU_EDGE;
    videoCodecContext->codec = codec;
    avcodec_open(videoCodecContext,codec);
    
    AVFrame * picture = avcodec_alloc_frame();
    int got_pitcure;
    AVPacket pkt;
    av_init_packet( &pkt );
    pkt.data = (uint8_t*)buf;
    pkt.size = len;
    
    printf("!!!!%d %04x %d",videoCodecContext->codec->type,videoCodecContext->flags);
    avcodec_decode_video2(videoCodecContext, picture, &got_pitcure, &pkt);
    printf("got_picture %d %d %d",got_pitcure,videoCodecContext->width,videoCodecContext->height);
}

int write_audio_frame(unsigned char *buf,double timeStamp,int len)
{
    if (_hasWriteOver) {
        return  -1;
    }
    if (!_hasCreateavcC) {
        return -1;
    }
    AVPacket packet;
    av_init_packet(&packet);
    packet.flags |= AV_PKT_FLAG_KEY;
    packet.stream_index = AUDIO_STREAM_INDEX;
    packet.data = (uint8_t*)buf;
    if (_fristTimeStamp == 0) {
        _fristTimeStamp = timeStamp;
    }
    packet.pts = timeStamp -_fristTimeStamp;
    packet.dts = packet.pts;
    packet.size = len;
    
    AVStream *audioStream = _formatContext->streams[AUDIO_STREAM_INDEX];
    
    int rc = aac_adtstoasc_filter(&_bsfc, audioStream->codec,
                                  NULL,
                                  &packet.data, &packet.size,
                                  buf, len,
                                  packet.flags & AV_PKT_FLAG_KEY);
    
    if (rc >= 0) {
        _M;if (av_interleaved_write_frame(_formatContext, &packet)) {
            fprintf(stderr, "Error while writing audio frame\n");
            av_free_packet(&packet);
            return  -1;
        }
    }
    av_free_packet(&packet);
    return 0;
    
}

static int write_avcc(unsigned char *p,int len)
{
    
    AVStream *stream = _formatContext->streams[VIDEO_STREAM_INDEX];
    unsigned char *b = (unsigned char*)p;
    
    /*判断第一个数据包是SPS，0x67*/
    if (b[4] != 0x67) {
        return 0;
    }
    
    
    //这里写入SPS,和PPS 作为MP4 box的avcc
    //第2，第3位要填写整个包的长度
    // len = ppsRawLength + spsRawLength
    int ppsRawLength = 8;
    int spsLength = len - ppsRawLength - 4;    //8的长度是pps的长度,4是首个00 00 00 01
    
    /*转换sps pps 数据包的头*/
    b[3] = (0xFF & spsLength);
    b[2] = (0xFF & (spsLength>> 8)); //高位
    b[len - 5] = 0x04;
    
    if (!_hasCreateavcC) {
        /*填写avcc信息
         *每个MP4 有且仅有一个 moov BOX 用来包含 avcc 的 BOX;
         */
        AVCodecContext *videoContext = stream->codec;
        //填写SPS头
        videoContext->extradata = (uint8_t*)av_mallocz(len+3);
        videoContext->extradata_size = len+3;
        videoContext->extradata[0] = 0x01;
        videoContext->extradata[1] = b[4+1];
        videoContext->extradata[2] = b[4+2];
        videoContext->extradata[3] = b[4+3];
        videoContext->extradata[4] = 0xFC | 3;
        videoContext->extradata[5] = 0xE0 | 1;
        videoContext->extradata[6] = (spsLength >> 8) & 0x00ff;
        videoContext->extradata[7] = spsLength & 0x00ff;
        //填写SPS数据
        int i=0;
        for (i ;i<spsLength;i++){
            videoContext->extradata[8+i] = b[4+i];
        }
        //填写PPS数据头
        int ppsLength = ppsRawLength - 4;
        videoContext->extradata[8+spsLength] = 0x01;
        videoContext->extradata[8+spsLength+1] = (ppsLength >> 8) & 0x00ff;
        videoContext->extradata[8+spsLength+2] = ppsLength & 0x00ff;
        //填写PPS数据
        for (i=0;i<ppsLength;i++){
            videoContext->extradata[8+spsLength+3+i] = b[spsLength + 4 + 4 +i];
        }
        for (i = 0; i< videoContext->extradata_size; i ++) {
            printf("%02x ",videoContext->extradata[i]);
        }
        printf("\n");
        _hasCreateavcC = true;
    }
}


static int write_1_nalu_video_frame(unsigned char * p,int len ,int spsPPSOffset ,long timeStamp)
{
    if (_hasWriteOver) {
        return -1;
    }
    
    AVStream *stream = _formatContext->streams[VIDEO_STREAM_INDEX];
    if (!stream) {
        return -1;
    }
    unsigned char *b = (unsigned char*)p;
    
    /*修改视频帧的帧头*/
    b[3 + spsPPSOffset] = (0xFF & (len - 4 - spsPPSOffset));
    b[2 + spsPPSOffset] = (0xFF & ((len - 4 - spsPPSOffset) >> 8));
    printf("write data len %d %02x%02x\n",len,b[2],b[3]);
    //__android_log_print(ANDROID_LOG_INFO, "jni", "test2 %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x"
    //		,p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],p[16],p[17],p[18],p[19],p[20],p[21],p[22],p[23],p[24],p[25],p[26],p[27],p[28],p[29],p[30],p[31]
    //		,p[32],p[33],p[34],p[35],p[36],p[37],p[38],p[39],p[40],p[41],p[42],p[43],p[44],p[45],p[46],p[47]);
    
    // Init packet
    AVPacket pkt;
    av_init_packet( &pkt );
    pkt.flags |= ( 0 >= getVopType( p + spsPPSOffset, len ) ) ? AV_PKT_FLAG_KEY : 0; //关键帧
    pkt.stream_index = stream->index;
    pkt.data = (uint8_t*)b;
    pkt.size = len;


    
    // Wait for key frame
    if ( waitkey ) {
        if ( 0 == ( pkt.flags & AV_PKT_FLAG_KEY ) )
            return 1;
        else
            waitkey = 0;
    }
    
    //The last one interleaves packets according to their timestamps, and the first one doesn't, it assumes they are already correctly interleaved.
    int ret = av_interleaved_write_frame( _formatContext, &pkt );
    if (ret != 0) {
        printf("write_1_nalu_video_frame:av_interleaved_write_frame error %d",ret);
    }
    av_free_packet(&pkt);
    return 0;
}

int close_output_file()
{
    _hasWriteOver = true;
    int ret;
    waitkey = 1;
    
    if ( !_formatContext )
        return -1;
    
    
    _M; ret = av_write_trailer(_formatContext );
    
    if (_formatContext->oformat && !(_formatContext->oformat->flags & AVFMT_NOFILE ) &&_formatContext->pb )
        avio_close(_formatContext->pb );
    
    // Free the stream
    _M; av_free(_formatContext );
    
    _formatContext = 0;
    return ret;
}




int  create_output_file( char *fileName,struct mp4_info_st mp4Info)
{
    
    int ret;
    _hasCreateavcC = false;
    _hasWriteHead = false;
    _hasWriteOver = false;
    av_log_set_level( AV_LOG_ERROR );
    av_register_all();
    // Create container
    _formatContext = avformat_alloc_context();
    _formatContext->video_codec_id = CODEC_ID_MPEG4;
    _M; AVOutputFormat *outputFormat = av_guess_format( 0, fileName, 0 );
    outputFormat->video_codec = CODEC_ID_H264;
    outputFormat->audio_codec = CODEC_ID_AAC;
    _formatContext->oformat = outputFormat;
    strcpy( _formatContext->filename, fileName );
    
    
    // Add video stream
    _M;AVStream *videoStream = avformat_new_stream(_formatContext, NULL);
//    AVStream *videoStream = av_new_stream( _formatContext,VIDEO_STREAM_INDEX );
    videoStream->index = VIDEO_STREAM_INDEX;
    videoStream->sample_aspect_ratio.den  = 11;
    videoStream->sample_aspect_ratio.num  = 16;
    if (!videoStream) {
        printf("create_output_file:Could not alloc video stream\n");
        return -1;
    }
    AVCodecContext *videoCodecContext = videoStream->codec;
    _M; avcodec_get_context_defaults3(videoCodecContext, NULL);
    videoCodecContext->codec_type    = AVMEDIA_TYPE_VIDEO;
    videoCodecContext->codec_id      = CODEC_ID_H264;
    videoCodecContext->time_base.den = mp4Info.videoFrameRate;
    videoCodecContext->time_base.num = 1;
    videoCodecContext->gop_size      = mp4Info.videoGopSize;
//    videoCodecContext->flags        |= CODEC_FLAG_GLOBAL_HEADER; //不一定有用
    videoCodecContext->sample_aspect_ratio.den  = 11;
    videoCodecContext->sample_aspect_ratio.num  = 16;
    
//    videoCodecContext->bit_rate      = 384000;
    
    //Add audio steam
    AVStream * audioStream = av_new_stream(_formatContext, AUDIO_STREAM_INDEX);
    if (!audioStream) {
        printf("create_output_file:Could not alloc audio stream\n");
        return -1;
    }
    audioStream->index = AUDIO_STREAM_INDEX;
    AVCodecContext *audioCodecContext = audioStream->codec;
    audioCodecContext->codec_id = CODEC_ID_AAC;
    audioCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    audioCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    audioCodecContext->bit_rate = mp4Info.AudioBitRate;
    audioCodecContext->sample_rate = mp4Info.AudioSimpleRate;
    audioCodecContext->channels = mp4Info.AudioChannel;
    
    
    return 0;
}

int write_video_fame(unsigned char *buf,double timeStamp,int len)
{
    int result;
    long time =(long) timeStamp / 6;
    int i;
    //for(i = 0 ; i < 100 ; i++){
    //__android_log_print(ANDROID_LOG_INFO, "jni", "test %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x"
    //		,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],buf[30],buf[31]
    //		,buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],buf[40],buf[41],buf[42],buf[43],buf[44],buf[45],buf[46],buf[47]);
    //}
    int ret = spareSPSPPS(buf,len);

    if (ret == 0 && _hasCreateavcC) {
        //写入数据
        result = write_1_nalu_video_frame(buf, len,0,time);
    } else {
        if (!_hasWriteHead) {
            result = init_video_codecContext(buf,ret - 8);
            _hasWriteHead = true;
        }
        //第一帧是sps
        //写入SPS加PPS
        result = write_avcc(buf, ret);
        //写入数据
        result = write_1_nalu_video_frame(buf, len,ret,time);
    }
    return result;
}
