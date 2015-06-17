//
//  mp4_file_muxer.h
//  GLCameraRipple
//
//  Created by howell on 14-7-11.
//
//

#ifndef GLCameraRipple_mp4_file_muxer_h
#define GLCameraRipple_mp4_file_muxer_h

typedef struct mp4_info_st{
    int AudioSimpleRate;
    int AudioChannel;
    int AudioBitRate;
    int videoFrameRate;
    int videoGopSize;
}mp4_info_st;


/**
 * 创建一个MP4文件
 *
 * @param fileName  生成的mp4文件完整路径，以.mp4后缀名结尾
 * @param mp4Info   包含音频信息的结构
 *
 * @return 0 on success, negative -1 on failure.
 */
int create_output_file( char *fileName,struct mp4_info_st mp4Info);

/**
 * 完整MP4文件写操作
 *
 * @return 0 on success, negative -1 on failure.
 */

int close_output_file();

/**
 * 写一帧视频数据，
 *
 * @param buf 完整的一帧h264数据，每个哪路以AennexB的序列排列
 * @param len 数据长度
 * @param timeStamp 时标
 *
 * @return 0 on success, negative -1 on failure.
 */
int write_video_fame(unsigned char *buf,double timeStamp,int len);

/**
 * 写一帧音频数据，
 *
 * @param buf 完整的一帧带有adts头的aac数据
 * @param len 数据长度
 * @param timeStamp 时标
 *
 * @return 0 on success, negative -1 on failure.
 */
int write_audio_frame(unsigned char *buf,double timeStamp,int len);


#endif
