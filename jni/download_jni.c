#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <jni.h>
#include <pthread.h>
#include<assert.h>
#include<sys/time.h>
#include<time.h>
#include "hwplay/stream_type.h"
#include "stream_buf_for_download.h"
#include "mp4_file_muxer.h"
#include "hw_config.h"

pthread_mutex_t lock_put,lock_get;
int BUF_LEN = 1024 * 1024;
int OUT_BUF = 1024 * 1024;
stream_buf* sb;
mp4_info_st mp4_info;
char* out;
char* final_out;
stream_head* out_head;

typedef struct
{
	const char* nalu;
	int size;
	unsigned long timestamp;
}nalu_t;

int find_nalu(nalu_t *nalus, int *count, const char *buf, int len, unsigned long timestamp)
{
	const char* nalu_beg = buf;
	const char* nalu_end = nalu_beg + 4;

	const char* nalu_buf;//保存nalu
	int nalu_len;//nalu的长度
	int nalu_type;//nalu的类型

	int nalu_count = 0;
	while(nalu_end  < (buf + len - 4)  )
	{
		if(nalu_end[0] == 0x00
				&& nalu_end[1] == 0x00
				&& nalu_end[2] == 0x00
				&& nalu_end[3] == 0x01)
		{
			//每个nalu的头4个字节为0x00,0x00,0x00,0x01
			nalu_buf = nalu_beg;
			nalu_len = nalu_end - nalu_beg;
			nalu_type = nalu_beg[4] & 0x1f;//nalu的第5个字节的前5bit为类型

			//TODO 可以处理nalu
			if(nalu_type == 0x05 /*i slice*/
					|| nalu_type == 0x01 /* p slice*/
					/*|| nalu_type == 0x06*/ /*sei*/
					|| nalu_type == 0x07 /*sps*/
					|| nalu_type == 0x08 /*pps*/
					/*|| nalu_type == 0x09*/ /*aud*/)
			{
				nalus[nalu_count].nalu = nalu_buf;
				nalus[nalu_count].timestamp = timestamp;
				nalus[nalu_count].size = nalu_len;
#if 0
				if(nalu_type == 0x07)
				{
					printf("sps %02x%02x%02x\n",nalus[nalu_count].nalu[1],nalus[nalu_count].nalu[2],nalus[nalu_count].nalu[3]);
					char* fsps = new char[nalus[nalu_count].size / 3 * 4 + 4 + 4];
					base64encode(fsps,(unsigned char*)nalus[nalu_count].nalu,nalus[nalu_count].size);
					printf("fsps %s\n",fsps);
					delete[] fsps;
				}
				if(nalu_type == 0x08)
				{
					char* fpps = new char[nalus[nalu_count].size / 3 * 4 + 4 + 4];
					base64encode(fpps,(unsigned char*)nalus[nalu_count].nalu,nalus[nalu_count].size);
					printf("fpps %s\n",fpps);
					delete[] fpps;
				}
#endif
				nalu_count++;
			}

			//处理完毕,继续寻找下一个nalu
			nalu_beg = nalu_end;
			nalu_end = nalu_beg + 4;

			nalu_type = nalu_beg[4] & 0x1f;
			if(nalu_type == 0x05
					|| nalu_type == 0x01)
			{
				//当类型为i slice 或p slice时肯定是最后一个，
				//节省搜索时间
				break;
			}
		}else{
			if(nalu_end[3] != 0)
			{
				nalu_end += 4;
			}
			else if(nalu_end[2] != 0)
			{
				nalu_end += 3;
			}
			else if(nalu_end[1] != 0)
			{
				nalu_end += 2;
			}
			else
			{
				nalu_end += 1;
			}
		}
	}

	//处理最后一个nalu
	nalu_buf = nalu_beg;
	nalu_end = buf + len;
	nalu_len = nalu_end - nalu_beg;
	nalu_type = nalu_beg[4] & 0x1f;

	//TODO 可以处理nalu
	if(nalu_type == 0x05 /*i slice*/
			|| nalu_type == 0x01 /* p slice*/
			/*|| nalu_type == 0x06*/ /*sei*/
			|| nalu_type == 0x07 /*sps*/
			|| nalu_type == 0x08 /*pps*/
			/*|| nalu_type == 0x09*/ /*aud*/)
	{
		nalus[nalu_count].nalu = nalu_buf;
		nalus[nalu_count].timestamp = timestamp;
		nalus[nalu_count].size = nalu_len;
		nalu_count++;
	}
	*count = nalu_count;
	return 0;
}

void parse_test(const char* in,int in_len,char* out,int out_len,int* ret_len,int time_stamp)
{
    nalu_t nalu_info[10];
    int nalu_count = 0;
    find_nalu(nalu_info,&nalu_count,in,in_len,time_stamp);

    char* dst = out;
    int dst_len = 0;
    int i;
    for(i =0 ; i < nalu_count; i++)
    {
        if(dst_len + nalu_info[i].size < out_len)
        {
			__android_log_print(ANDROID_LOG_INFO, "jni", "nalu_info[i].size:%d,nalu_count:%d",nalu_info[i].size,nalu_count);
            memcpy(dst,nalu_info[i].nalu,nalu_info[i].size);
            dst += nalu_info[i].size;
            dst_len += nalu_info[i].size;
        }
    }

    *ret_len = dst_len;
}

void input_stream(const char* buf,unsigned int len){
	//pthread_mutex_lock(&lock_put);
	int ret;
	int out_len;
	int final_out_len;
	if(stream_buf_input_data(sb,buf,len) == 0){//成功
		while(get_one_frame_stream(out,&out_len,out_head) == 1){
			if(out_head->type == 0 || out_head->type == 1){
				parse_test(out,out_len ,final_out,1024*1024,&final_out_len,out_head->time_stamp);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------final_out_len:%d",final_out_len);
				ret = write_video_fame(final_out,out_head->time_stamp,final_out_len);
				struct tm* tm = gmtime(&out_head->sys_time);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------%04d %02d %02d %02d %02d %02d----",tm->tm_year + 1900,tm->tm_mon + 1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------write_video_fame ret = %d ----",ret);
			}else  if(out_head->type == 2){
				ret = write_audio_frame(out,out_head->time_stamp * 8 / 1000 ,out_len);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------write_audio_frame ret = %d,out_len=%d----",ret,out_len);
			}
		}
	}
	//out_len = OUT_BUF;
	//pthread_mutex_unlock(&lock_put);
}

void download_init(char* file_name){
	//pthread_mutex_init(&lock_put,NULL);
	//pthread_mutex_init(&lock_get,NULL);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_init 1");
	mp4_info.AudioSimpleRate = 8000;
	mp4_info.AudioChannel = 1;
	mp4_info.AudioBitRate = 12000;
	mp4_info.videoFrameRate = 25;
	mp4_info.videoGopSize = 100;
	__android_log_print(ANDROID_LOG_INFO, "jni", "------file_name = %d ----",file_name);
	int ret = create_output_file(file_name,mp4_info);
	__android_log_print(ANDROID_LOG_INFO, "jni", "------create_output_file ret = %d ----",ret);
	sb = stream_buf_open(BUF_LEN);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_init 2");
	out_head = (stream_head*)malloc(sizeof(stream_head));
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_init 3");
	out = (char*)malloc(OUT_BUF);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_init 4");
	final_out = (char*)malloc(OUT_BUF);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_init 5");
}

void download_deinit(){
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 1");
	close_output_file();
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 2");
	stream_buf_close(sb);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 3");
	sb = NULL;
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 4");
	free(out_head);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 5");
	out_head = NULL;
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 6");
	free(out);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 7");
	free(final_out);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "download_deinit 8");
	out = NULL;

}

int get_one_frame_stream(char* out,unsigned int* out_len,stream_head* out_head){
	if(out_len == NULL
			|| out_head == NULL
			|| out == NULL)
	{
		return -1;
	}
	//pthread_mutex_lock(&lock_get);

	stream_head head;
	int ret = stream_buf_copy_data(sb,(char*)&head,sizeof(head));
	if(ret < 0)
	{
		return -1;
	}

#if 1
	if(head.tag != HW_MEDIA_TAG)
	{
		//数据异常，过滤
		char temp;
		do
		{
			stream_buf_get_data(sb,&temp,1);
			if(stream_buf_copy_data(sb,(char*)&head,sizeof(head)) != 0)
			{
				//stream buf中没有sizeof(head)的数据
				return -1;
			}
		}while(head.tag != HW_MEDIA_TAG);
	}
#endif

	if(head.len > OUT_BUF)
	{
		return -1;
	}
	int data_len = 0;
	ret = stream_buf_get_data_len(sb,&data_len);
	if(data_len < head.len)
	{
		return -1;
	}
	ret = stream_buf_get_data(sb,out_head,sizeof(stream_head));
	if(ret < 0)
	{
		return -1;
	}
	ret = stream_buf_get_data(sb,out,out_head->len - sizeof(stream_head));
	if(ret < 0)
	{
		return -1;
	}

	*out_len = out_head->len - sizeof(stream_head);
	return 1;
#if 0
	__android_log_print(ANDROID_LOG_INFO, "jni", "5");
	int data_len = 0;
	ret = stream_buf_get_data_len(sb,&data_len);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ret:%d , data_len:%d ,head.len:%d",ret,data_len,head.len);
	if(data_len >= head.len
			&& *out_len >= head.len - sizeof(stream_head))
	{
		__android_log_print(ANDROID_LOG_INFO, "jni", "6");
		*out_len = head.len - sizeof(stream_head);
		__android_log_print(ANDROID_LOG_INFO, "jni", "7 out_len:%d",out_len);
		stream_buf_get_data(sb,(char*)out_head,sizeof(stream_head));
		__android_log_print(ANDROID_LOG_INFO, "jni", "8");
		stream_buf_get_data(sb,out,*out_len);
		__android_log_print(ANDROID_LOG_INFO, "jni", "9");
		return 1;
	}
#endif
	//pthread_mutex_unlock(&lock_get);
}
