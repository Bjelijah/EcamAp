#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <jni.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <pthread.h>
#include <semaphore.h>

#include "hwplay/stream_type.h"
#include "hwplay/play_def.h"

#include <string.h>
#include <time.h>
#include "net_sdk.h"
#include<assert.h>
#include<sys/time.h>
#include "stream_buf_for_download.h"
#include "mp4_file_muxer.h"
#include "hw_config.h"






#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "yv12", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "yv12", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "yv12", __VA_ARGS__))



struct YV12glDisplay
{
	char * y;
	char * u;
	char * v;
	unsigned long long time;
	int width;
	int height;
	//int inited;
	int enable;
	//int is_catch_picture;
	//char path[50];

	/* multi thread */
	int method_ready;
	JavaVM * jvm;
	JNIEnv * env;
	jmethodID mid,mSetTime;
	jobject obj;
	pthread_mutex_t lock;
	unsigned long long first_time;
	//sem_t over_sem;
	//sem_t over_ret_sem;
};

static struct YV12glDisplay self;

void yuv12gl_set_enable(int enable)
{
	self.enable = enable;
	self.method_ready = 0;
}

void yv12gl_display(const unsigned char * y, const unsigned char *u,const unsigned char *v, int width, int height, unsigned long long time)
{
	//LOGE("display");
	//LOGE("yv12gl_display timestamp: %llu",time);
	if (!self.enable) return;
	self.time = time/1000;
	//LOGE("progress self.time :%llu",self.time);
	if(self.jvm->AttachCurrentThread( &self.env, NULL) != JNI_OK) {
		LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
		return;
	}

	/* get JAVA method first */
	if (!self.method_ready) {
		//LOGE("111111111");

		jclass cls = self.env->GetObjectClass(self.obj);
		//self.clz = (*self.env)->FindClass(self.env, "com/howell/webcam/player/YV12Renderer");
		if (cls == NULL) {
			LOGE("FindClass() Error.....");
			goto error;
		}
		//�ٻ�����еķ���
		self.mid = self.env->GetMethodID(cls, "requestRender", "()V");
		self.mSetTime = self.env->GetMethodID( cls, "setTime", "(J)V");
		if (self.mid == NULL || self.mSetTime == NULL) {
			LOGE("GetMethodID() Error.....");
			goto error;
		}
		self.method_ready=1;
	}

	// LOGE("22222222");
	self.env->CallVoidMethod(self.obj,self.mSetTime,self.time);
	/*
  if (sem_trywait(&self.over_sem)==0) {
	  if (self.method_ready)
	  {

	  }
	  sem_post(&self.over_ret_sem);
	  self.enable=0;
	  return;
  }
	 */
	//LOGE("33333333");
	pthread_mutex_lock(&self.lock);
	if (width!=self.width || height!=self.height) {
		self.y = realloc(self.y,width*height);
		self.u = realloc(self.u,width*height/4);
		self.v = realloc(self.v,width*height/4);
		self.width = width;
		self.height = height;
	}
	memcpy(self.y,y,width*height);
	memcpy(self.u,u,width*height/4);
	memcpy(self.v,v,width*height/4);
	pthread_mutex_unlock(&self.lock);

	//LOGE("4444444");
	/* notify the JAVA */
	self.env->CallVoidMethod( self.obj, self.mid, NULL);
	//LOGE("555555555");

	if (self.jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
	return;

	error:
	if (self.jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
	return;
}



JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeInit
(JNIEnv *env, jclass, jobject obj){
	env->GetJavaVM(&self.jvm);
	self.obj = env->NewGlobalRef(obj);
	pthread_mutex_init(&self.lock,NULL);
	self.width = 352;
	self.height = 288;
	self.y = malloc(self.width*self.height);
	self.u = malloc(self.width*self.height/4);
	self.v = malloc(self.width*self.height/4);
	memset(self.y,0,self.width*self.height);
	memset(self.u,128,self.width*self.height/4);
	memset(self.v,128,self.width*self.height/4);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeOnSurfaceCreated
(JNIEnv *, jclass){
	self.enable=1;
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeRenderY
(JNIEnv *, jclass){
	pthread_mutex_lock(&self.lock);
	if (self.y == NULL) {
		char value[4] = {0,0,0,0};
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,2,2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,value);
	}
	else {
		//LOGI("render y");
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,self.width,self.height,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,self.y);
	}
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeRenderU
(JNIEnv *, jclass){
	if (self.u == NULL) {
		char value[] = {128};
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,1,1,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,value);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,self.width/2,self.height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,self.u);
	}
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeRenderV
(JNIEnv *, jclass){
	if (self.v==NULL) {
		char value[] = {128};
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,1,1,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,value);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,self.width/2,self.height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,self.v);
	}
	pthread_mutex_unlock(&self.lock);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeDeinit
(JNIEnv *, jclass){
	self.method_ready = 0;
	free(self.y);
	free(self.u);
	free(self.v);
}


struct AudioPlay
{
	/* multi thread */
	int method_ready;
	JavaVM * jvm;
	JNIEnv * env;
	jmethodID mid;
	jobject obj;
	jfieldID data_length_id;
	jbyteArray data_array;
	int data_array_len;

	int stop;
	sem_t over_audio_sem;
	sem_t over_audio_ret_sem;
};
static struct AudioPlay audioSelf;

void audio_stop()
{
	audioSelf.stop=1;
}


void audio_play(const char* buf,int len,int au_sample,int au_channel,int au_bits)
{
	if (audioSelf.stop) return;
	if (audioSelf.jvm->AttachCurrentThread( &audioSelf.env, NULL) != JNI_OK) {
		LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
		return;
	}
	if (!audioSelf.method_ready) {
		jclass cls;
		cls = audioSelf.env->GetObjectClass(audioSelf.obj);
		if (cls == NULL) {
			LOGE("FindClass() Error.....");
			goto error;
		}
		audioSelf.mid = audioSelf.env->GetMethodID( cls, "audioWrite", "()V");
		if (audioSelf.mid == NULL) {
			LOGE("GetMethodID() Error.....");
			goto error;
		}
		audioSelf.method_ready=1;
	}
	audioSelf.env->SetIntField(audioSelf.obj,audioSelf.data_length_id,len);
	if (len<=audioSelf.data_array_len) {
		audioSelf.env->SetByteArrayRegion(audioSelf.data_array,0,len,(const signed char*)buf);
		audioSelf.env->CallVoidMethod( audioSelf.obj, audioSelf.mid, NULL);
	}
	if (audioSelf.jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
	return;
	error:
	if (audioSelf.jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeAudioInit
(JNIEnv *env, jclass, jobject obj){
	env->GetJavaVM(&audioSelf.jvm);
	audioSelf.obj = env->NewGlobalRef(obj);
	jclass clz = env->GetObjectClass( obj);
	audioSelf.data_length_id = env->GetFieldID(clz, "mAudioDataLength", "I");
	jfieldID id = env->GetFieldID(clz,"mAudioData","[B");
	jbyteArray data_array_local_ref = (jbyteArray)env->GetObjectField(obj,id);
	int data_array_len_local_ref = env->GetArrayLength(data_array_local_ref);
	jbyteArray data_array_global_ref = (jbyteArray)env->NewGlobalRef( data_array_local_ref);
	audioSelf.data_array = data_array_global_ref;
	audioSelf.data_array_len = data_array_len_local_ref;
	sem_init(&audioSelf.over_audio_sem,0,0);
	sem_init(&audioSelf.over_audio_ret_sem,0,0);
	audioSelf.method_ready = 0;
	audioSelf.stop = 0;
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeAudioStop
(JNIEnv *, jclass){
	audio_stop();
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_nativeAudioDeinit
(JNIEnv *, jclass){
	//TODO
}

static int total_file_list_count;
struct StreamResource
{
	PLAY_HANDLE play_handle;
	LIVE_STREAM_HANDLE live_stream_handle;
	FILE_STREAM_HANDLE file_stream_handle;
	FILE_STREAM_HANDLE download_file_stream_handle;
	FILE_LIST_HANDLE file_list_handle;
	USER_HANDLE user_handle;
	int is_playback;
	int media_head_len;
	size_t stream_len;

	JavaVM * jvm;
	JNIEnv * env;
	jmethodID mid;
	jobject obj;
	pthread_mutex_t lock_put;

	FILE *foutput;	//下载文件
	int is_exit;	//退出标记位
	//int is_mediahead_ready;
	//int row,col;
	//int color_mode;
	//int year,month,day,hour,minute,second;
};
static struct StreamResource * res = NULL;
static int is_first_stream = 0;

void on_live_stream_fun(LIVE_STREAM_HANDLE handle,int stream_type,const char* buf,int len,long userdata){
	//__android_log_print(ANDROID_LOG_INFO, "jni", "-------------stream_type %d-len %d",stream_type,len);
	res->stream_len += len;
	int ret = hwplay_input_data(res->play_handle, buf ,len);
}

void on_file_stream_fun(FILE_STREAM_HANDLE handle,const char* buf,int len,long userdata){
	res->stream_len += len;
	int ret = hwplay_input_data(res->play_handle, buf ,len);
	while(ret <= 0 && res->is_exit == 0){
		usleep(10000);
		ret = hwplay_input_data(res->play_handle, buf ,len);
	}
}



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
		while(get_one_frame_stream(out,(unsigned int*)&out_len,out_head) == 1){
			if(out_head->type == 0 || out_head->type == 1){
				parse_test(out,out_len ,final_out,1024*1024,&final_out_len,out_head->time_stamp);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------final_out_len:%d",final_out_len);
				ret = write_video_fame((unsigned char*)final_out,out_head->time_stamp,final_out_len);
				struct tm* tm = gmtime(&out_head->sys_time);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------%04d %02d %02d %02d %02d %02d----",tm->tm_year + 1900,tm->tm_mon + 1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
				__android_log_print(ANDROID_LOG_INFO, "jni", "------write_video_fame ret = %d ----",ret);
			}else  if(out_head->type == 2){
				ret = write_audio_frame((unsigned char *)out,out_head->time_stamp * 8 / 1000 ,out_len);
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
	ret = stream_buf_get_data(sb,(char *)out_head,sizeof(stream_head));
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




void call_java_method_refreshDataLen(int len){//FIXME
	if (res->jvm->AttachCurrentThread( &res->env, NULL) != JNI_OK) {
		LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
		return;
	}
	/* get JAVA method first */
	if (is_first_stream == 0) {
		jclass cls;
		cls = res->env->GetObjectClass(res->obj);
		if (cls == NULL) {
			LOGE("FindClass() Error.....");
			goto error;
		}
		//�ٻ�����еķ���
		res->mid = res->env->GetMethodID( cls, "refreshDataLen", "(I)V");
		if (res->mid == NULL) {
			LOGE("GetMethodID() Error.....");
			goto error;
		}
		is_first_stream = 1;
	}
	/* notify the JAVA */
	res->env->CallVoidMethod( res->obj, res->mid, len);
	if (res->jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
	return;
	error:
	if (res->jvm->DetachCurrentThread() != JNI_OK) {
		LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
}


void on_download_file_stream_fun_ex(FILE_STREAM_HANDLE handle,const char* buf,int len,long userdata){
	pthread_mutex_lock(&res->lock_put);
	__android_log_print(ANDROID_LOG_INFO, "jni", "len:%d",len);
	input_stream(buf,len);
#if 1
	call_java_method_refreshDataLen(len);//FIXME
#endif
	pthread_mutex_unlock(&res->lock_put);
}


static void on_yuv_callback_ex(PLAY_HANDLE handle,
		const unsigned char* y,
		const unsigned char* u,
		const unsigned char* v,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		unsigned long long time,
		long user)
{
	yv12gl_display(y,u,v,width,height,time);
}


static void on_source_callback(PLAY_HANDLE handle,
		int type,//0-��Ƶ,1-��Ƶ
		const char* buf,//��ݻ���,�������Ƶ����ΪYV12��ݣ��������Ƶ��Ϊpcm���
		int len,//��ݳ���,���Ϊ��Ƶ��Ӧ�õ���w * h * 3 / 2
		unsigned long timestamp,//ʱ��,��λΪ����
		long sys_tm,//osd ʱ��(1970�����ڵ�UTCʱ��)
		int w,//��Ƶ��,��Ƶ�����Ч
		int h,//��Ƶ��,��Ƶ�����Ч
		int framerate,//��Ƶ֡��,��Ƶ�����Ч
		int au_sample,//��Ƶ������,��Ƶ�����Ч
		int au_channel,//��Ƶͨ����,��Ƶ�����Ч
		int au_bits,//��Ƶλ��,��Ƶ�����Ч
		long user)
{
	if(type == 0){//音频
		audio_play(buf,len,au_sample,au_channel,au_bits);
	}else if(type == 1){//视频
		unsigned char* y = (unsigned char *)buf;
		unsigned char* u = y+w*h;
		unsigned char* v = u+w*h/4;
		yv12gl_display(y,u,v,w,h,timestamp);
	}
}

static void on_audio_callback(PLAY_HANDLE handle,
		const char* buf,//数据缓存,如果是视频，则为YV12数据，如果是音频则为pcm数据
		int len,//数据长度,如果为视频则应该等于w * h * 3 / 2
		unsigned long timestamp,//时标,单位为毫秒
		long user){
	audio_play(buf,len,0,0,0);
}

int login(const char* ip){
	hwplay_init(1,0,0);
	int ret = hwnet_init(5888);
	/* 192.168.128.83 */
	int user_handle = hwnet_login(ip,5198,"admin","12345");
	if(user_handle == -1){
		return -1;
	}
	return user_handle;
}

PLAY_HANDLE init_play_handle(int slot,int is_playback ,SYSTEMTIME beg,SYSTEMTIME end){
	RECT area ;
	HW_MEDIAINFO media_head;
	memset(&media_head,0,sizeof(media_head));
	if(!is_playback){//预览
		res->live_stream_handle = hwnet_get_live_stream(res->user_handle,slot,1,0,on_live_stream_fun,0);
		__android_log_print(ANDROID_LOG_INFO, "jni", "live_stream_handle: %d",res->live_stream_handle);
		__android_log_print(ANDROID_LOG_INFO, "jni", "1");
		//int media_head_len = 0;
		int ret2 = hwnet_get_live_stream_head(res->live_stream_handle,(char*)&media_head,1024,&res->media_head_len);
		__android_log_print(ANDROID_LOG_INFO, "jni", "ret2 :%d adec_code %x",ret2,media_head.adec_code);
		__android_log_print(ANDROID_LOG_INFO, "jni", "is_playback :%d",is_playback);
	}else{//回放
		__android_log_print(ANDROID_LOG_INFO, "jni", "is_playback :%d",is_playback);
		file_stream_t file_info;
		res->file_stream_handle = hwnet_get_file_stream(res->user_handle,slot,beg,end,on_file_stream_fun,0,&file_info);
		__android_log_print(ANDROID_LOG_INFO, "jni", "file_stream_handle: %d total_len:%d",res->file_stream_handle,file_info.len);
		int b = hwnet_get_file_stream_head(res->file_stream_handle,(char*)&media_head,1024,&res->media_head_len);
		//media_head.adec_code = 0xa;
		__android_log_print(ANDROID_LOG_INFO, "jni", "hwnet_get_file_stream_head ret:%d",b);
	}
	PLAY_HANDLE  ph = hwplay_open_stream((char*)&media_head,sizeof(media_head),1024*1024,is_playback,area);
	hwplay_open_sound(ph);
	hwplay_set_max_framenum_in_buf(ph,is_playback?25:5);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "ph is:%d",ph);
	//	int b= hwplay_register_yuv_callback_ex(ph,on_yuv_callback_ex,0);
	int b = hwplay_register_source_data_callback(ph,on_source_callback,0);
	//__android_log_print(ANDROID_LOG_INFO, "JNI", "hwplay_register_source_data_callback:%d",b);
	//	hwplay_register_audio_callback(ph,on_audio_callback,0);
	b = hwplay_play(ph);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "b:%d",b);
	return ph;
}

void on_download_file_stream_fun(FILE_STREAM_HANDLE handle,const char* buf,int len,long userdata){
	__android_log_print(ANDROID_LOG_INFO, "on_download_file_stream_fun", "len:%d",len);
	pthread_mutex_lock(&res->lock_put);
	while(res->foutput == NULL){
		//__android_log_print(ANDROID_LOG_INFO, "on_download_file_stream_fun", "sleep");
		sleep(0.5);
	}
	if(res->foutput != NULL){
		__android_log_print(ANDROID_LOG_INFO, "on_download_file_stream_fun write", "len:%d",len);
		int ret = fwrite(buf, sizeof(unsigned char), len, res->foutput);
		//__android_log_print(ANDROID_LOG_INFO, "fwrite_ret", "size:%d",ret);
#if 1
		__android_log_print(ANDROID_LOG_INFO, "on_download_file_stream_fun", "11");
		call_java_method_refreshDataLen(len);
		__android_log_print(ANDROID_LOG_INFO, "on_download_file_stream_fun", "22");
#endif
	}
	pthread_mutex_unlock(&res->lock_put);
}


PLAY_HANDLE init_download_play_handle(int user_handle,int slot ,SYSTEMTIME beg,SYSTEMTIME end,char* file_name){
	RECT area ;
	HW_MEDIAINFO media_head;
	memset(&media_head,0,sizeof(media_head));
	//__android_log_print(ANDROID_LOG_INFO, "jni", "is_playback :%d",is_playback);
	file_stream_t file_info;
	res->download_file_stream_handle = hwnet_get_file_stream(user_handle,slot,beg,end,on_download_file_stream_fun,0,&file_info);
	if(res->download_file_stream_handle == -1){
		return -1;
	}
	//res->file_stream_handle = hwnet_get_file_stream(res->user_handle,slot,beg,end,on_file_stream_fun,0,&file_info);
	LOGI("download_file_stream_handle: %d total_len:%d",res->download_file_stream_handle,file_info.len);
	int ret = hwnet_get_file_stream_head(res->download_file_stream_handle,(char*)&media_head,1024,&res->media_head_len);
	//media_head.adec_code = 0xa;
	LOGI("jni", "hwnet_get_file_stream_head ret:%d",ret);
	//res->is_mediahead_ready = 1;
	//LOGE("file_name:%s",file_name);
	FILE *foutput = fopen(file_name, "wb");
	LOGE("111");
	fwrite(&media_head, sizeof(media_head), 1, foutput);
	LOGE("222");
	fclose(foutput);
	LOGE("333");
	FILE *foutput_again = fopen(file_name, "ab");
	LOGE("444");
	res->foutput = foutput_again;
	return file_info.len;
}

static void create_resource()
{
	res = (struct StreamResource *)calloc(1,sizeof(*res));
	is_first_stream = 0;
	res->stream_len = 0;
	res->is_exit = 0;
	if (res == NULL) return;
}


JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_downloadInitEx
(JNIEnv *env, jclass, jobject obj, jstring j_file_name, jint slot, jshort begYear, jshort begMonth, jshort begDay, jshort begHour, jshort begMinute, jshort begSecond, jshort endYear, jshort endMonth, jshort endDay, jshort endHour, jshort endMinute, jshort endSecond){
	env->GetJavaVM(&res->jvm);
	res->obj = env->NewGlobalRef(obj);
	const char* file_name = env-> GetStringUTFChars(j_file_name,NULL);
	download_init((char *)file_name);
	env->ReleaseStringUTFChars(j_file_name,file_name);
	pthread_mutex_init(&res->lock_put,NULL);

	SYSTEMTIME beg;
	beg.wYear = begYear;
	beg.wMonth = begMonth;
	beg.wDay = begDay;
	beg.wHour = begHour;
	beg.wMinute = begMinute;
	beg.wSecond = begSecond;
	SYSTEMTIME end;
	end.wYear = endYear;
	end.wMonth = endMonth;
	end.wDay = endDay;
	end.wHour = endHour;
	end.wMinute = endMinute;
	end.wSecond = endSecond;
	RECT area ;
	HW_MEDIAINFO media_head;
	memset(&media_head,0,sizeof(media_head));
	file_stream_t file_info;
	memset(&file_info,0,sizeof(file_info));
	res->download_file_stream_handle = hwnet_get_file_stream(res->user_handle,slot,beg,end,on_download_file_stream_fun,0,&file_info);
	return file_info.len;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_downloadInit
(JNIEnv *env, jclass, jobject obj, jint user_handle, jstring j_file_name, jint slot, jshort begYear, jshort begMonth, jshort begDay, jshort begHour, jshort begMinute, jshort begSecond, jshort endYear, jshort endMonth, jshort endDay, jshort endHour, jshort endMinute, jshort endSecond){
	create_resource();
	env->GetJavaVM(&res->jvm);
	res->obj = env->NewGlobalRef(obj);
	const char* file_name = env-> GetStringUTFChars(j_file_name,NULL);
	//download_init(file_name);
	LOGE("filename:%s",file_name);
	pthread_mutex_init(&res->lock_put,NULL);

	SYSTEMTIME beg;
	beg.wYear = begYear;
	beg.wMonth = begMonth;
	beg.wDay = begDay;
	beg.wHour = begHour;
	beg.wMinute = begMinute;
	beg.wSecond = begSecond;
	SYSTEMTIME end;
	end.wYear = endYear;
	end.wMonth = endMonth;
	end.wDay = endDay;
	end.wHour = endHour;
	end.wMinute = endMinute;
	end.wSecond = endSecond;
	RECT area ;

	int total_len = init_download_play_handle(user_handle,slot,beg,end,(char *)file_name);
	env->ReleaseStringUTFChars(j_file_name,file_name);
	//HW_MEDIAINFO media_head;
	//memset(&media_head,0,sizeof(media_head));
	//file_stream_t file_info;
	//memset(&file_info,0,sizeof(file_info));
	//res->download_file_stream_handle = hwnet_get_file_stream(res->user_handle,slot,beg,end,on_download_file_stream_fun,0,&file_info);
	return total_len;
}


JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_downloadDestory
(JNIEnv *env, jclass){
	if(res->foutput != NULL){
		fclose(res->foutput);
		res->foutput = NULL;
	}
	hwnet_close_file_stream(res-> download_file_stream_handle);
	//hwplay_stop(res->download_play_handle);
	//hwnet_release();
	env->DeleteGlobalRef(res->obj);
	//	env->DeleteGlobalRef( res->foutput);
	free(res);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_downloadDestoryEx
(JNIEnv *, jclass){
	download_deinit();
	int ret = hwnet_close_file_stream(res-> download_file_stream_handle);
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_cameraLogin
(JNIEnv *env, jclass, jstring j_ip){
	__android_log_print(ANDROID_LOG_INFO, "!!!", "login");
	const char* ip = env-> GetStringUTFChars(j_ip,NULL);
	__android_log_print(ANDROID_LOG_INFO, "!!!", "ip %s",ip);
	//create_resource();
	int ret = login(ip);
	env->ReleaseStringUTFChars(j_ip,ip);
	return ret;
}


JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_display
(JNIEnv *env, jclass, jint slot, jint is_playback, jshort begYear, jshort begMonth, jshort begDay, jshort begHour, jshort begMinute, jshort begSecond, jshort endYear, jshort endMonth, jshort endDay, jshort endHour, jshort endMinute, jshort endSecond){
	__android_log_print(ANDROID_LOG_INFO, "!!!", "display slot:%d",slot);
	res->is_playback = is_playback;
	SYSTEMTIME beg;
	SYSTEMTIME end;
	if(is_playback == 0){
		res->play_handle = init_play_handle(slot,is_playback,beg,end);
	}else{
		beg.wYear = begYear;
		beg.wMonth = begMonth;
		beg.wDay = begDay;
		beg.wHour = begHour;
		beg.wMinute = begMinute;
		beg.wSecond = begSecond;

		end.wYear = endYear;
		end.wMonth = endMonth;
		end.wDay = endDay;
		end.wHour = endHour;
		end.wMinute = endMinute;
		end.wSecond = endSecond;
		__android_log_print(ANDROID_LOG_INFO, "decod_jni", "test :%d-%d-%d %d:%d:%d\n"
				,beg.wYear, beg.wMonth,beg.wDay,beg.wHour,beg.wMinute,beg.wSecond);
		res->play_handle = init_play_handle(slot,is_playback,beg,end);
	}
	return res->play_handle;
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_quit
(JNIEnv *, jclass){
	__android_log_print(ANDROID_LOG_INFO, "quit", "1");
	res->is_exit = 1;
	int ret;
	if(!res->is_playback){
		ret = hwnet_close_live_stream(res->live_stream_handle);
	}else{
		ret = hwnet_close_file_stream(res-> file_stream_handle);
	}
	__android_log_print(ANDROID_LOG_INFO, "quit", "2");
	if(ret == 0){
		__android_log_print(ANDROID_LOG_INFO, "quit", "close stream fail");
		return;
	}
	__android_log_print(ANDROID_LOG_INFO, "quit", "3");
	ret = hwnet_logout(res->user_handle);
	__android_log_print(ANDROID_LOG_INFO, "quit", "4");
	if(ret == 0){
		__android_log_print(ANDROID_LOG_INFO, "quit", "logout fail");
		return;
	}
	__android_log_print(ANDROID_LOG_INFO, "quit", "5");
	hwplay_stop(res->play_handle);
	__android_log_print(ANDROID_LOG_INFO, "quit", "6");
	//hwnet_release();
	__android_log_print(ANDROID_LOG_INFO, "quit", "7");
	free(res);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_changeToD1
  (JNIEnv *, jclass){
	int ret = hwnet_close_live_stream(res->live_stream_handle);
	if(ret == 0){
		return;
	}
	res->live_stream_handle = hwnet_get_live_stream(res->user_handle,0,1,0,on_live_stream_fun,0);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_changeTo720P
  (JNIEnv *, jclass){
	int ret = hwnet_close_live_stream(res->live_stream_handle);
	if(ret == 0){
		return;
	}
	res->live_stream_handle = hwnet_get_live_stream(res->user_handle,0,0,0,on_live_stream_fun,0);
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_getReplayListCount
  (JNIEnv *, jclass){
	 time_t nowtime;
	 struct tm *timeinfo;
	 time( &nowtime );
	 timeinfo = localtime( &nowtime );
	 int year, month, day,hour,min,second;
	 year = timeinfo->tm_year + 1900;
	 month = timeinfo->tm_mon + 1;
	 day = timeinfo->tm_mday;
	 hour = timeinfo->tm_hour;
	 min = timeinfo->tm_min;
	 second = timeinfo->tm_sec;
	 SYSTEMTIME end;
	 end.wYear = year;
	 end.wMonth = month;
	 end.wDay = day;
	 end.wHour = hour;
	 end.wMinute = min;
	 end.wSecond = second;
	 __android_log_print(ANDROID_LOG_INFO, "jni", "end:%4d-%02d-%02d %02d:%02d:%02d\n"
	 			,end.wYear, end.wMonth,
	 			end.wDay,end.wHour,end.wMinute,end.wSecond);
	 SYSTEMTIME beg;
	 beg.wYear = year;
	 if(month - 2 >= 1){
		 beg.wMonth = month - 2;
	 }else if(month == 2){
		 beg.wYear = year - 1;
		 beg.wMonth = 12;
	 }else if(month == 1){
		 beg.wYear = year - 1;
		 beg.wMonth = 11;
	 }
	 beg.wDay = day ;
	 if(end.wMonth == 2 && end.wDay > 28){
		 beg.wDay = 28;
	 }
	 beg.wHour = hour;
	 beg.wMinute = min;
	 beg.wSecond = second;
	 __android_log_print(ANDROID_LOG_INFO, "jni", "beg:%4d-%02d-%02d %02d:%02d:%02d\n"
		 			,beg.wYear, beg.wMonth,
		 			beg.wDay,beg.wHour,beg.wMinute,beg.wSecond);
	__android_log_print(ANDROID_LOG_INFO, "jni", "%4d-%02d-%02d %02d:%02d:%02d\n"
			,1900+timeinfo->tm_year, 1+timeinfo->tm_mon,
			timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	__android_log_print(ANDROID_LOG_INFO, "jni", "hwnet_get_file_list 1");
	int file_list_handle = hwnet_get_file_list(res->user_handle,0,beg,end,0);
	__android_log_print(ANDROID_LOG_INFO, "jni", "hwnet_get_file_list 2");
	//int count;
	int ret = hwnet_get_file_count(file_list_handle,&total_file_list_count);
	__android_log_print(ANDROID_LOG_INFO, "jni", "file_list_handle %d,count %d ret %d\n",file_list_handle,total_file_list_count,ret);
	return file_list_handle;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_catchPicture
  (JNIEnv *env, jclass, jstring j_path){
	const char* path = env-> GetStringUTFChars(j_path,NULL);
	int ret = hwplay_save_to_jpg(res->play_handle,path,70);
	env->ReleaseStringUTFChars(j_path,path);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_setFlip
  (JNIEnv *, jclass){
	int is_flip;
	int ret = hwnet_get_flip(res->user_handle,0,&is_flip);
	ret = hwnet_enable_flip(res->user_handle,0,!is_flip);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_getStreamLen
  (JNIEnv *, jclass){
	return res->stream_len;
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_playbackPause
  (JNIEnv *, jclass, jboolean bPause){
	int ret = hwplay_pause(res->play_handle,bPause);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_playBackPositionChange
  (JNIEnv *, jclass, jshort begYear, jshort begMonth, jshort begDay, jshort begHour, jshort begMinute, jshort begSecond , jshort endYear, jshort endMonth,
		  jshort endDay, jshort endHour, jshort endMinute, jshort endSecond){
	SYSTEMTIME beg;
		SYSTEMTIME end;
		beg.wYear = begYear;
		beg.wMonth = begMonth;
		beg.wDay = begDay;
		beg.wHour = begHour;
		beg.wMinute = begMinute;
		beg.wSecond = begSecond;

		end.wYear = endYear;
		end.wMonth = endMonth;
		end.wDay = endDay;
		end.wHour = endHour;
		end.wMinute = endMinute;
		end.wSecond = endSecond;
		__android_log_print(ANDROID_LOG_INFO, "decod_jni", "test :%d-%d-%d %d:%d:%d\n"
					,beg.wYear, beg.wMonth,beg.wDay,beg.wHour,beg.wMinute,beg.wSecond);
		int ret = hwnet_close_file_stream(res-> file_stream_handle);
		hwplay_clear_stream_buf(res->play_handle);
		file_stream_t file_info;
		res->file_stream_handle = hwnet_get_file_stream(res->user_handle,0,beg,end,on_file_stream_fun,0,&file_info);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_ptzTurnLeft
  (JNIEnv *, jclass, jint slot){

	ptz_ctrl_t ctrl;
	ctrl.slot = slot;
	ctrl.control = 0;
	ctrl.cmd = 4;//Left
	ctrl.value = 64;
	int ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnLeft ret:%d",ret);
	usleep(800*1000);
	ctrl.cmd = 5;//Stop
	ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnStop ret:%d",ret);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_ptzTurnRight
  (JNIEnv *, jclass, jint slot){
	ptz_ctrl_t ctrl;
	ctrl.slot = slot;
	ctrl.control = 0;
	ctrl.cmd = 6;//Right
	ctrl.value = 64;
	int ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnRight ret:%d",ret);
	usleep(800*1000);
	ctrl.cmd = 5;//Stop
	ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnStop ret:%d",ret);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_ptzTurnUp
  (JNIEnv *, jclass, jint slot){
	ptz_ctrl_t ctrl;
	ctrl.slot = slot;
	ctrl.control = 0;
	ctrl.cmd = 8;//Up
	ctrl.value = 64;
	int ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnUP ret:%d",ret);
	usleep(800*1000);
	ctrl.cmd = 5;//Stop
	ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnStop ret:%d",ret);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_ptzTurnDown
  (JNIEnv *, jclass, jint slot){

	ptz_ctrl_t ctrl;
	ctrl.slot = slot;
	ctrl.control = 0;
	ctrl.cmd = 2;//Down
	ctrl.value = 64;
	int ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnDown ret:%d",ret);
	usleep(800*1000);
	ctrl.cmd = 5;//Stop
	ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ptzTurnStop ret:%d",ret);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_zoomTele
  (JNIEnv *, jclass, jint slot){

	ptz_ctrl_t ctrl;
	ctrl.slot = slot;
	ctrl.control = 1;
	ctrl.cmd = 3;//tele
	ctrl.value = 64;
	int ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "zoomTele ret:%d",ret);
	usleep(800*1000);
	ctrl.cmd = 7;//Stop
	ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "stop ret:%d",ret);
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_zoomWide
  (JNIEnv *, jclass, jint slot){
	ptz_ctrl_t ctrl;
	ctrl.slot = slot;
	ctrl.control = 1;
	ctrl.cmd = 4;//wide
	ctrl.value = 64;
	int ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "zoomWide ret:%d",ret);
	usleep(800*1000);
	ctrl.cmd = 7;//Stop
	ret = hwnet_ptz_ctrl(res->user_handle, &ctrl);
	__android_log_print(ANDROID_LOG_INFO, "jni", "stop ret:%d",ret);
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_getWifi
  (JNIEnv * env, jclass, jstring j_ip){
	hwnet_init(5888);
	const char* ip = env-> GetStringUTFChars(j_ip,NULL);
	int user_handle = hwnet_login(ip,5198,"admin","12345");
	if(user_handle == -1){
		__android_log_print(ANDROID_LOG_INFO, "jni", "user_handle fail");
		return 0;
	}
	NetWlanAP info;
	int ret = hwnet_get_wifi(user_handle,&info);
	LOGE("info.ssid:%s,info.key:%s,info.channel:%s,info.flag:%d",info.ssid,info.key,info.channel,info.flag);
	//logout
	env->ReleaseStringUTFChars(j_ip,ip);
	hwnet_logout(user_handle);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_setWifi
  (JNIEnv *env, jclass, jstring j_ip, jstring j_ssid, jstring j_password){
	hwnet_init(5888);
	const char* ip = env-> GetStringUTFChars(j_ip,NULL);
	int user_handle = hwnet_login(ip,5198,"admin","12345");
	if(user_handle == -1){
		__android_log_print(ANDROID_LOG_INFO, "jni", "user_handle fail");
		return 0;
	}else{
		__android_log_print(ANDROID_LOG_INFO, "jni", "user_handle success");
	}
	//set wifi info
	const char* ssid = env-> GetStringUTFChars(j_ssid,NULL);
	const char* password = env-> GetStringUTFChars(j_password,NULL);
	NetWlanAP info;
	info.flag = 3;
	strcpy(info.ssid,ssid);
	strcpy(info.key,password);
	int ret = hwnet_set_wifi(user_handle,&info);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ret:%d",ret);
	//free resource
	env->ReleaseStringUTFChars(j_ip,ip);
	env->ReleaseStringUTFChars(j_ssid,ssid);
	env->ReleaseStringUTFChars(j_password,password);
	//logout
	hwnet_logout(user_handle);
	return ret;
}

JNIEXPORT jobjectArray JNICALL Java_com_howell_jni_JniUtil_getReplayList
  (JNIEnv * env, jclass, jint file_list_handle, jint count){
	__android_log_print(ANDROID_LOG_INFO, "jni", "getReplayList file_list_handle:%d",file_list_handle);
	int i ,type;
	SYSTEMTIME beg ,end;

	jobjectArray MXArray = NULL;       // jobjectArray 为指针类型
	jclass clsMX = NULL;         // jclass 为指针类型
	jobject obj;

	//知道要返回的class.
	clsMX = env->FindClass("com/howell/ecameraap/ReplayFile");

	//创建一个MXAray的数组对象.
	MXArray = env->NewObjectArray(count, clsMX, NULL);

	//获取类中每一个变量的定义
	jfieldID begYear = env->GetFieldID(clsMX, "begYear", "S");
	jfieldID begMonth = env->GetFieldID(clsMX, "begMonth", "S");
	jfieldID begDay = env->GetFieldID(clsMX, "begDay", "S");
	jfieldID begHour = env->GetFieldID(clsMX, "begHour", "S");
	jfieldID begMinute = env->GetFieldID(clsMX, "begMinute", "S");
	jfieldID begSecond = env->GetFieldID(clsMX, "begSecond", "S");

	jfieldID endYear = env->GetFieldID(clsMX, "endYear", "S");
	jfieldID endMonth = env->GetFieldID(clsMX, "endMonth", "S");
	jfieldID endDay = env->GetFieldID(clsMX, "endDay", "S");
	jfieldID endHour = env->GetFieldID(clsMX, "endHour", "S");
	jfieldID endMinute = env->GetFieldID(clsMX, "endMinute", "S");
	jfieldID endSecond = env->GetFieldID(clsMX, "endSecond", "S");
	//得到这个类的构造方法id.  //得到类的默认构造方法的id.都这样写.
	jmethodID consID = env->GetMethodID(clsMX, "<init>", "()V");
	int j = 0;
	for (i = 0; i < count; i++)
	{
		memset(&beg,0,sizeof(SYSTEMTIME));
		memset(&end,0,sizeof(SYSTEMTIME));
		int ret = hwnet_get_file_detail(file_list_handle,i,&beg,&end,&type);//1成功 0失败
		if(ret == 1){
			__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d\n",ret);
			__android_log_print(ANDROID_LOG_INFO, "jni", "%4d-%02d-%02d %02d:%02d:%02d -> %4d-%02d-%02d %02d:%02d:%02d\n"
				,beg.wYear, beg.wMonth,beg.wDay,beg.wHour,beg.wMinute,beg.wSecond
				,end.wYear, end.wMonth,end.wDay,end.wHour,end.wMinute,end.wSecond);
			obj = env->NewObject(clsMX, consID);
			env->SetShortField(obj, begYear, beg.wYear);
			env->SetShortField(obj, begMonth, beg.wMonth);
			env->SetShortField(obj, begDay, beg.wDay);
			env->SetShortField(obj, begHour, beg.wHour);
			env->SetShortField(obj, begMinute, beg.wMinute);
			env->SetShortField(obj, begSecond, beg.wSecond);

			env->SetShortField(obj, endYear, end.wYear);
			env->SetShortField(obj, endMonth, end.wMonth);
			env->SetShortField(obj, endDay, end.wDay);
			(env)->SetShortField(obj, endHour, end.wHour);
			(env)->SetShortField(obj, endMinute, end.wMinute);
			(env)->SetShortField(obj, endSecond, end.wSecond);
			(env)->SetObjectArrayElement(MXArray, j, obj);
			j++;
		}else{
			break;
		}
		__android_log_print(ANDROID_LOG_INFO, "jni", "j size:%d",j);
	}

	if(j < count){
		MXArray = NULL;
		MXArray = env->NewObjectArray(j, clsMX, NULL);
		int temp = j;
		j = 0;
		for (i = 0; i < temp; i++)
		{
			memset(&beg,0,sizeof(SYSTEMTIME));
			memset(&end,0,sizeof(SYSTEMTIME));
			int ret = hwnet_get_file_detail(file_list_handle,i,&beg,&end,&type);//1成功 0失败
			if(ret == 1){
				__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d\n",ret);
				__android_log_print(ANDROID_LOG_INFO, "jni", "%4d-%02d-%02d %02d:%02d:%02d -> %4d-%02d-%02d %02d:%02d:%02d\n"
						,beg.wYear, beg.wMonth,beg.wDay,beg.wHour,beg.wMinute,beg.wSecond
						,end.wYear, end.wMonth,end.wDay,end.wHour,end.wMinute,end.wSecond);
				obj = env->NewObject(clsMX, consID);
				(env)->SetShortField(obj, begYear, beg.wYear);
				(env)->SetShortField(obj, begMonth, beg.wMonth);
				(env)->SetShortField(obj, begDay, beg.wDay);
				(env)->SetShortField(obj, begHour, beg.wHour);
				(env)->SetShortField(obj, begMinute, beg.wMinute);
				(env)->SetShortField(obj, begSecond, beg.wSecond);

				(env)->SetShortField(obj, endYear, end.wYear);
				(env)->SetShortField(obj, endMonth, end.wMonth);
				(env)->SetShortField(obj, endDay, end.wDay);
				(env)->SetShortField(obj, endHour, end.wHour);
				(env)->SetShortField(obj, endMinute, end.wMinute);
				(env)->SetShortField(obj, endSecond, end.wSecond);
				(env)->SetObjectArrayElement(MXArray, j, obj);
				j++;
			}else{
				break;
			}
			__android_log_print(ANDROID_LOG_INFO, "jni", "j size:%d",j);
		}
	}
	return MXArray;
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_closeFileList
  (JNIEnv *, jclass, jint file_list_handle){
	int ret = hwnet_close_file_list(file_list_handle);
	__android_log_print(ANDROID_LOG_INFO, "jni", "close list ret %d\n",ret);
}


SYSTEMTIME get_replay_end_systime(JNIEnv *env,jobject replay){
	SYSTEMTIME end;
	jclass objectClass = (env)->GetObjectClass(replay);
	if(objectClass == NULL)
	{
			LOGE("GetObjectClass failed \n");
			return end;
	}
	jfieldID endYearFieldID = (env)->GetFieldID(objectClass,"endYear","S");
	jfieldID endMonthFieldID = (env)->GetFieldID(objectClass,"endMonth","S");
	jfieldID endDayFieldID = (env)->GetFieldID(objectClass,"endDay","S");
	jfieldID endHourFieldID = (env)->GetFieldID(objectClass,"endHour","S");
	jfieldID endMinuteFieldID = (env)->GetFieldID(objectClass,"endMinute","S");
	jfieldID endSecondFieldID = (env)->GetFieldID(objectClass,"endSecond","S");
	end.wYear = (env)->GetShortField( replay , endYearFieldID);
	end.wMonth = (env)->GetShortField( replay , endMonthFieldID);
	end.wDay = (env)->GetShortField( replay , endDayFieldID);
	end.wHour = (env)->GetShortField( replay , endHourFieldID);
	end.wMinute = (env)->GetShortField( replay , endMinuteFieldID);
	end.wSecond = (env)->GetShortField( replay , endSecondFieldID);
	 __android_log_print(ANDROID_LOG_INFO, "jni", "end:%4d-%02d-%02d %02d:%02d:%02d\n"
		 			,end.wYear, end.wMonth,
		 			end.wDay,end.wHour,end.wMinute,end.wSecond);
	return end;
}

SYSTEMTIME get_replay_beg_systime(JNIEnv *env,jobject replay){
	SYSTEMTIME beg;
	jclass objectClass = (env)->GetObjectClass(replay);
	if(objectClass == NULL)
	{
		LOGE("GetObjectClass failed \n");
		return beg;
	}
	jfieldID begYearFieldID = (env)->GetFieldID(objectClass,"begYear","S");
	jfieldID begMonthFieldID = (env)->GetFieldID(objectClass,"begMonth","S");
	jfieldID begDayFieldID = (env)->GetFieldID(objectClass,"begDay","S");
	jfieldID begHourFieldID = (env)->GetFieldID(objectClass,"begHour","S");
	jfieldID begMinuteFieldID = (env)->GetFieldID(objectClass,"begMinute","S");
	jfieldID begSecondFieldID = (env)->GetFieldID(objectClass,"begSecond","S");
	beg.wYear = (env)->GetShortField( replay , begYearFieldID);
	beg.wMonth = (env)->GetShortField( replay , begMonthFieldID);
	beg.wDay = (env)->GetShortField( replay , begDayFieldID);
	beg.wHour = (env)->GetShortField( replay , begHourFieldID);
	beg.wMinute = (env)->GetShortField( replay , begMinuteFieldID);
	beg.wSecond = (env)->GetShortField( replay , begSecondFieldID);
	 __android_log_print(ANDROID_LOG_INFO, "jni", "beg:%4d-%02d-%02d %02d:%02d:%02d\n"
			 			,beg.wYear, beg.wMonth,
			 			beg.wDay,beg.wHour,beg.wMinute,beg.wSecond);
	return beg;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_getListByPage
  (JNIEnv *env, jclass, jint user_handle, jint slot, jint stream, jobject replay, jint type, jint order_by_time, jobject page_info){
	//time_t nowtime;
	//struct tm *timeinfo;
	//time( &nowtime );
	//timeinfo = localtime( &nowtime );
	SYSTEMTIME end = get_replay_end_systime(env,replay);
	SYSTEMTIME beg = get_replay_beg_systime(env,replay);
	jclass objectClass = env->GetObjectClass(page_info);
	if(objectClass == NULL)
	{
		LOGE("GetObjectClass failed \n");
		return -1;
	}
	jfieldID pageSizeFieldID = (env)->GetFieldID(objectClass,"page_size","I");
	jfieldID pageNoFieldID = 	(env)->GetFieldID(objectClass,"page_no","I");

	jint page_size = (env)->GetIntField( page_info , pageSizeFieldID);
	jint page_no = (env)->GetIntField( page_info , pageNoFieldID);
	LOGE("page_size:%d ,page_no:%d",page_size,page_no);
	Pagination page;
	page.page_size = page_size;
	page.page_no = page_no;
	int file_list_handle =  hwnet_get_file_list_by_page(user_handle,slot,stream, beg, end, type, order_by_time,0,&page);
	if(file_list_handle == -1){
		LOGE("hwnet_get_file_list_by_page failed \n");
		return -1;
	}

	//获取类中每一个变量的定义
	jfieldID totalSizeFieldID = (env)->GetFieldID(objectClass,"total_size","I");
	jfieldID curSizeFieldID = (env)->GetFieldID(objectClass,"cur_size","I");
	jfieldID pageCountFieldID = (env)->GetFieldID(objectClass,"page_count","I");

	(env)->SetIntField( page_info,totalSizeFieldID,page.total_size);
	(env)->SetIntField( page_info,curSizeFieldID,page.cur_size);
	(env)->SetIntField( page_info,pageCountFieldID,page.page_count);
	LOGE("page.total_size:%d ,page.cur_size:%d ,page.page_count:%d",page.total_size,page.cur_size,page.page_count);
	return file_list_handle;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_vedioListLogin
  (JNIEnv *env, jclass, jstring j_ip){
	 __android_log_print(ANDROID_LOG_INFO, "!!!", "login");
	const char* ip = (env)-> GetStringUTFChars(j_ip,NULL);
	 __android_log_print(ANDROID_LOG_INFO, "!!!", "ip %s",ip);
	 //create_resource();
	int ret = login(ip);
	(env)->ReleaseStringUTFChars(j_ip,ip);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_howell_jni_JniUtil_vedioListLogout
  (JNIEnv *, jclass, jint user_handle){
	int ret = hwnet_logout(user_handle);
	//free(res);
	return ret;
}

JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_displayLocalFile
  (JNIEnv *env, jclass, jstring fileName){
	hwplay_init(1,0,0);
	create_resource();
	const char* path = env-> GetStringUTFChars(fileName,NULL);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "path is:%s",path);
	res->play_handle = hwplay_open_local_file(path);
	(env)->ReleaseStringUTFChars(fileName,path);
	hwplay_open_sound(res->play_handle);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "ph is:%d",res->play_handle);
//	hwplay_register_yuv_callback_ex(res->play_handle,on_yuv_callback_ex,0);
//	hwplay_register_audio_callback(res->play_handle,on_audio_callback,0);
	int b = hwplay_register_source_data_callback(res->play_handle,on_source_callback,0);
	hwplay_play(res->play_handle);

}


JNIEXPORT void JNICALL Java_com_howell_jni_JniUtil_localFileQuit
  (JNIEnv *, jclass){
	hwplay_stop(res->play_handle);
	free(res);
}

