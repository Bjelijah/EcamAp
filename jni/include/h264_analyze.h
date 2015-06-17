//
//  h264_analyze.h
//  GLCameraRipple
//
//  Created by howell on 14-7-10.
//
//

#ifndef GLCameraRipple_h264_analyze_h
#define GLCameraRipple_h264_analyze_h



void get_pic_info_fromSPS(unsigned char * buf,int len, int * width ,int *height , int *framerate);
#endif
