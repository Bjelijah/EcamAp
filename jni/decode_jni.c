#include <jni.h>
#include <pthread.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h> 
#include <time.h>

#include "hwplay/stream_type.h"
#include "hwplay/play_def.h"

#include "net_sdk.h"

static int total_file_list_count;
struct StreamResource
{
	PLAY_HANDLE play_handle;
	LIVE_STREAM_HANDLE live_stream_handle;
	FILE_STREAM_HANDLE file_stream_handle;
	FILE_LIST_HANDLE file_list_handle;
	USER_HANDLE user_handle;
	int is_playback;
	int media_head_len;
	size_t stream_len;

	//int row,col;
	//int color_mode;
	//int year,month,day,hour,minute,second;
};
static struct StreamResource * res = NULL;

void on_live_stream_fun(LIVE_STREAM_HANDLE handle,int stream_type,const char* buf,int len,long userdata){
	//__android_log_print(ANDROID_LOG_INFO, "jni", "-------------stream_type %d-len %d",stream_type,len);
	res->stream_len += len;
	int ret = hwplay_input_data(res->play_handle, buf ,len);
	//if(ret == 0)
	//{
		// hwplay_get_stream_buf_len(
	//	__android_log_print(ANDROID_LOG_INFO, "jni", "hwplay_input_data error");
	//}

	int buf_len;
	ret = hwplay_get_stream_buf_remain(res->play_handle,&buf_len);
	if(ret == 1)
	{
		__android_log_print(ANDROID_LOG_INFO, "jni", "buf_len %d",buf_len);
	}
	//if(buf_len < 1000000){
	//	__android_log_print(ANDROID_LOG_INFO, "jni", "buf_len less than 1000000");
	//	hwplay_clear_stream_buf(res->play_handle);
	//}
}

void on_file_stream_fun(FILE_STREAM_HANDLE handle,const char* buf,int len,long userdata){
	res->stream_len += len;
	int ret = hwplay_input_data(res->play_handle, buf ,len);
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
	//__android_log_print(ANDROID_LOG_INFO, "jni", "start decode  time: %llu",time);
	//sdl_display_input_data(y,u,v,width,height,time);

	yv12gl_display(y,u,v,width,height,time);
}

on_source_callback(PLAY_HANDLE handle,
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
  //__android_log_print(ANDROID_LOG_INFO, "JNI", "type:%d len:%d %lu\n",type,len,timestamp);

  if (type == 0) {
    audio_play(buf,len,au_sample,au_channel,au_bits);
  } 
  /*else if (type == 1) {
    native_catch_picture(res->play_handle);
  }
  __android_log_print(ANDROID_LOG_INFO, "JNI", "type0 over");*/
}

on_audio_callback(PLAY_HANDLE handle,
		const char* buf,//数据缓存,如果是视频，则为YV12数据，如果是音频则为pcm数据
		int len,//数据长度,如果为视频则应该等于w * h * 3 / 2
		unsigned long timestamp,//时标,单位为毫秒
		long user){
	__android_log_print(ANDROID_LOG_INFO, "audio", "on_audio_callback timestamp: %lu ",timestamp);

	//if(res[user]->is_exit == 1) return;
	audio_play(buf,len,0,0,0);

}

int login(const char* ip){
	//__android_log_print(ANDROID_LOG_INFO, "jni", "start init ph palyback: %d",is_playback);
	//hwplay_init(1,352,288);
	hwplay_init(1,0,0);
	__android_log_print(ANDROID_LOG_INFO, "jni", "0");

	int ret = hwnet_init(5888);
	
	/* 192.168.128.83 */
	res->user_handle = hwnet_login(ip,5198,"admin","12345");
	if(res->user_handle == -1){ 
		__android_log_print(ANDROID_LOG_INFO, "jni", "user_handle fail");
		return -1;
	}
	__android_log_print(ANDROID_LOG_INFO, "jni", "user_handle: %d",res->user_handle);
	return 0;
}

static PLAY_HANDLE init_play_handle(int is_playback ,SYSTEMTIME beg,SYSTEMTIME end){
	RECT area ;
	HW_MEDIAINFO media_head;
	memset(&media_head,0,sizeof(media_head));
	if(!is_playback){//预览
		res->live_stream_handle = hwnet_get_live_stream(res->user_handle,0,0,0,on_live_stream_fun,0);
		__android_log_print(ANDROID_LOG_INFO, "jni", "live_stream_handle: %d",res->live_stream_handle);
		__android_log_print(ANDROID_LOG_INFO, "jni", "1");
		//int media_head_len = 0;
		int ret2 = hwnet_get_live_stream_head(res->live_stream_handle,(char*)&media_head,1024,&res->media_head_len);
		__android_log_print(ANDROID_LOG_INFO, "jni", "ret2 :%d adec_code %x",ret2,media_head.adec_code);
		__android_log_print(ANDROID_LOG_INFO, "jni", "is_playback :%d",is_playback);
	}else{//回放
		__android_log_print(ANDROID_LOG_INFO, "jni", "is_playback :%d",is_playback);
		file_stream_t file_info;
		res->file_stream_handle = hwnet_get_file_stream(res->user_handle,0,beg,end,on_file_stream_fun,0,&file_info);
		__android_log_print(ANDROID_LOG_INFO, "jni", "file_stream_handle: %d",res->file_stream_handle);
		int b = hwnet_get_file_stream_head(res->file_stream_handle,(char*)&media_head,1024,&res->media_head_len);
		//media_head.adec_code = 0xa;
		__android_log_print(ANDROID_LOG_INFO, "jni", "hwnet_get_file_stream_head ret:%d",b);
	}
	PLAY_HANDLE  ph = hwplay_open_stream((char*)&media_head,sizeof(media_head),1024*1024,is_playback,area);
	hwplay_open_sound(ph);
	hwplay_set_max_framenum_in_buf(ph,is_playback?25:5);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "ph is:%d",ph);
	int b= hwplay_register_yuv_callback_ex(ph,on_yuv_callback_ex,0);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "hwplay_register_yuv_callback_ex :%d",b);
	//b = hwplay_register_source_data_callback(ph,on_source_callback,0);
	//__android_log_print(ANDROID_LOG_INFO, "JNI", "hwplay_register_source_data_callback:%d",b);
	hwplay_register_audio_callback(ph,on_audio_callback,0);
	b = hwplay_play(ph);
	__android_log_print(ANDROID_LOG_INFO, "JNI", "b:%d",b);
	return ph;
}

static void create_resource(JNIEnv *env, jobject obj)
{
  /* make sure init once */
  //__android_log_print(ANDROID_LOG_INFO, "!!!", "create_resource %d",is_playback);
  res = (struct StreamResource *)calloc(1,sizeof(*res));
  //total_file_list_count = 0;
  res->stream_len = 0;
  if (res == NULL) return;
}

JNIEXPORT int JNICALL Java_com_howell_ecameraap_HWCameraActivity_cameraLogin
(JNIEnv *env, jobject obj, jstring j_ip){
	 __android_log_print(ANDROID_LOG_INFO, "!!!", "login");
	const char* ip = (*env)-> GetStringUTFChars(env,j_ip,NULL);
	 __android_log_print(ANDROID_LOG_INFO, "!!!", "ip %s",ip);
	create_resource(env,obj);
	int ret = login(ip);
	(*env)->ReleaseStringUTFChars(env,j_ip,ip);
	return ret;
}

JNIEXPORT int JNICALL Java_com_howell_ecameraap_HWCameraActivity_display
(JNIEnv *env, jclass cls, jint is_playback,jshort begYear,jshort begMonth,jshort begDay,jshort begHour
		,jshort begMinute,jshort begSecond,jshort endYear,jshort endMonth,jshort endDay,jshort endHour,jshort endMinute
		,jshort endSecond){
	__android_log_print(ANDROID_LOG_INFO, "!!!", "display");
	res->is_playback = is_playback;
	SYSTEMTIME beg;
	SYSTEMTIME end;
	if(is_playback == 0){
		res->play_handle = init_play_handle(is_playback,beg,end);
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
		res->play_handle = init_play_handle(is_playback,beg,end);
	}
	return res->play_handle;
}

//�˳�
JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_quit
(JNIEnv *env, jclass cls){
	__android_log_print(ANDROID_LOG_INFO, "quit", "1");
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
	hwnet_release();
	__android_log_print(ANDROID_LOG_INFO, "quit", "7");
	free(res);
}

JNIEXPORT int JNICALL Java_com_howell_ecameraap_HWCameraActivity_catchPicture
(JNIEnv *env, jclass cls, jstring j_path){
	 __android_log_print(ANDROID_LOG_INFO, "jni", "catchPicture");
	const char* path = (*env)-> GetStringUTFChars(env,j_path,NULL);
	__android_log_print(ANDROID_LOG_INFO, "jni", "path %s res->play_handle %d",path,res->play_handle);
	int ret = hwplay_save_to_jpg(res->play_handle,path,70);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d",ret);
	(*env)->ReleaseStringUTFChars(env,j_path,path);
	return ret;
}

JNIEXPORT int JNICALL Java_com_howell_ecameraap_HWCameraActivity_setFlip
(JNIEnv *env, jclass cls){
	__android_log_print(ANDROID_LOG_INFO, "jni", "setFlip");
	int is_flip;
	int ret = hwnet_get_flip(res->user_handle,0,&is_flip);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d,is_flip %d",ret,is_flip);
	ret = hwnet_enable_flip(res->user_handle,0,!is_flip);
	__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d",ret);
	return ret;
}

JNIEXPORT int JNICALL Java_com_howell_ecameraap_HWCameraActivity_getStreamLen
(JNIEnv *env, jobject obj){
	__android_log_print(ANDROID_LOG_INFO, "jni", "getStreamLen");
	return res->stream_len;
}

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_changeToD1
(JNIEnv *env, jobject obj){
	__android_log_print(ANDROID_LOG_INFO, "jni", "changeToD1");
	__android_log_print(ANDROID_LOG_INFO, "quit", "1");
	int ret = hwnet_close_live_stream(res->live_stream_handle);
	__android_log_print(ANDROID_LOG_INFO, "quit", "2");
	if(ret == 0){
		__android_log_print(ANDROID_LOG_INFO, "quit", "close live stream fail");
		return;
	}
	__android_log_print(ANDROID_LOG_INFO, "quit", "3");
	res->live_stream_handle = hwnet_get_live_stream(res->user_handle,0,1,0,on_live_stream_fun,0);
	__android_log_print(ANDROID_LOG_INFO, "jni", "live_stream_handle: %d",res->live_stream_handle);
}

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_changeTo720P
(JNIEnv *env, jobject obj){
	__android_log_print(ANDROID_LOG_INFO, "jni", "changeTo720P");
	__android_log_print(ANDROID_LOG_INFO, "quit", "1");
	int ret = hwnet_close_live_stream(res->live_stream_handle);
	__android_log_print(ANDROID_LOG_INFO, "quit", "2");
	if(ret == 0){
		__android_log_print(ANDROID_LOG_INFO, "quit", "close live stream fail");
		return;
	}
	__android_log_print(ANDROID_LOG_INFO, "quit", "3");
	res->live_stream_handle = hwnet_get_live_stream(res->user_handle,0,0,0,on_live_stream_fun,0);
	__android_log_print(ANDROID_LOG_INFO, "jni", "live_stream_handle: %d",res->live_stream_handle);
}

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_playBackPositionChange
(JNIEnv *env, jobject obj,jshort begYear,jshort begMonth,jshort begDay,jshort begHour
		,jshort begMinute,jshort begSecond,jshort endYear,jshort endMonth,jshort endDay,jshort endHour,jshort endMinute
		,jshort endSecond){
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

JNIEXPORT int JNICALL Java_com_howell_ecameraap_HWCameraActivity_getReplayListCount
(JNIEnv *env, jclass cls){
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
	 SYSTEMTIME beg;
	 beg.wYear = year;
	 beg.wMonth = month;
	 beg.wDay = day;
	 beg.wHour = hour;
	 beg.wMinute = min;
	 beg.wSecond = second;
	 SYSTEMTIME end;
	 end.wYear = year;
	 if(month - 2 >= 1){
		 end.wMonth = month - 2;
	 }else if(month == 2){
		 end.wYear = year - 1;
		 end.wMonth = 12;
	 }else if(month == 1){
		 end.wYear = year - 1;
		 end.wMonth = 11;
	 }
	 end.wDay = day ;
	 if(end.wMonth == 2 && end.wDay > 28){
		 end.wDay = 28;
	 }
	 end.wHour = hour;
	 end.wMinute = min;
	 end.wSecond = second;
	__android_log_print(ANDROID_LOG_INFO, "jni", "%4d-%02d-%02d %02d:%02d:%02d\n"
			,1900+timeinfo->tm_year, 1+timeinfo->tm_mon,
			timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	int file_list_handle = hwnet_get_file_list(res->user_handle,0,end,beg,0);
	//int count;
	int ret = hwnet_get_file_count(file_list_handle,&total_file_list_count);
	__android_log_print(ANDROID_LOG_INFO, "jni", "count %d ret %d\n",total_file_list_count,ret);
	return file_list_handle;
}

JNIEXPORT jobjectArray JNICALL Java_com_howell_ecameraap_VedioList_getReplayList
(JNIEnv *env, jclass cls ,int file_list_handle,int count){
	__android_log_print(ANDROID_LOG_INFO, "jni", "getReplayList file_list_handle:%d",file_list_handle);
	int i ,type;
	SYSTEMTIME beg ,end;
	/*for(i = 0 ; i < count ; i++){
		int ret = hwnet_get_file_detail(file_list_handle,i,&beg,&end,&type);
		__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d\n",ret);
		__android_log_print(ANDROID_LOG_INFO, "jni", "%4d-%02d-%02d %02d:%02d:%02d -> %4d-%02d-%02d %02d:%02d:%02d\n"
		,beg.wYear, beg.wMonth,beg.wDay,beg.wHour,beg.wMinute,beg.wSecond
		,end.wYear, end.wMonth,end.wDay,end.wHour,end.wMinute,end.wSecond);
	}*/

	jobjectArray MXArray = NULL;       // jobjectArray 为指针类型
	jclass clsMX = NULL;         // jclass 为指针类型
	jobject obj;

	jint len = count;  //设置这个数组的长度.
	__android_log_print(ANDROID_LOG_INFO, "jni", "count1: %d\n total_file_list_count： %d",len,total_file_list_count);
	if(len > total_file_list_count){
		len = total_file_list_count;
		__android_log_print(ANDROID_LOG_INFO, "jni", "count2: %d\n",len);
	}

	//知道要返回的class.
	clsMX = (*env)->FindClass(env,"com/howell/ecameraap/ReplayFile");

	//创建一个MXAray的数组对象.
	MXArray = (*env)->NewObjectArray(env,len, clsMX, NULL);

	//获取类中每一个变量的定义
	jfieldID begYear = (*env)->GetFieldID(env,clsMX, "begYear", "S");
	jfieldID begMonth = (*env)->GetFieldID(env,clsMX, "begMonth", "S");
	jfieldID begDay = (*env)->GetFieldID(env,clsMX, "begDay", "S");
	jfieldID begHour = (*env)->GetFieldID(env,clsMX, "begHour", "S");
	jfieldID begMinute = (*env)->GetFieldID(env,clsMX, "begMinute", "S");
	jfieldID begSecond = (*env)->GetFieldID(env,clsMX, "begSecond", "S");

	jfieldID endYear = (*env)->GetFieldID(env,clsMX, "endYear", "S");
	jfieldID endMonth = (*env)->GetFieldID(env,clsMX, "endMonth", "S");
	jfieldID endDay = (*env)->GetFieldID(env,clsMX, "endDay", "S");
	jfieldID endHour = (*env)->GetFieldID(env,clsMX, "endHour", "S");
	jfieldID endMinute = (*env)->GetFieldID(env,clsMX, "endMinute", "S");
	jfieldID endSecond = (*env)->GetFieldID(env,clsMX, "endSecond", "S");
	//得到这个类的构造方法id.  //得到类的默认构造方法的id.都这样写.
	jmethodID consID = (*env)->GetMethodID(env,clsMX, "<init>", "()V");
	int j = 0;
	for (i = total_file_list_count - 1; i >= total_file_list_count - len; i--)
	{
		__android_log_print(ANDROID_LOG_INFO, "jni", "total_file_list_count - len： %d,i:%d\n",total_file_list_count - len,i);
		memset(&beg,0,sizeof(SYSTEMTIME));
		memset(&end,0,sizeof(SYSTEMTIME));
		int ret = hwnet_get_file_detail(file_list_handle,i,&beg,&end,&type);//1成功 0失败
		__android_log_print(ANDROID_LOG_INFO, "jni", "ret %d\n",ret);
		__android_log_print(ANDROID_LOG_INFO, "jni", "%4d-%02d-%02d %02d:%02d:%02d -> %4d-%02d-%02d %02d:%02d:%02d\n"
				,beg.wYear, beg.wMonth,beg.wDay,beg.wHour,beg.wMinute,beg.wSecond
				,end.wYear, end.wMonth,end.wDay,end.wHour,end.wMinute,end.wSecond);
			__android_log_print(ANDROID_LOG_INFO, "jni", "new onject");
			obj = (*env)->NewObject(env,clsMX, consID);
			(*env)->SetIntField(env,obj, begYear, beg.wYear);
			(*env)->SetIntField(env,obj, begMonth, beg.wMonth);
			(*env)->SetIntField(env,obj, begDay, beg.wDay);
			(*env)->SetIntField(env,obj, begHour, beg.wHour);
			(*env)->SetIntField(env,obj, begMinute, beg.wMinute);
			(*env)->SetIntField(env,obj, begSecond, beg.wSecond);

			(*env)->SetIntField(env,obj, endYear, end.wYear);
			(*env)->SetIntField(env,obj, endMonth, end.wMonth);
			(*env)->SetIntField(env,obj, endDay, end.wDay);
			(*env)->SetIntField(env,obj, endHour, end.wHour);
			(*env)->SetIntField(env,obj, endMinute, end.wMinute);
			(*env)->SetIntField(env,obj, endSecond, end.wSecond);
			(*env)->SetObjectArrayElement(env,MXArray, j, obj);
			j++;
	}
	return MXArray;

	/*返回java对象
	 * //获取Java中的mx2Data类
	jclass objectClass = (*env)->FindClass(env,"com/howell/ecameraap/ReplayFile");

	//得到这个类的构造方法id.  //得到类的默认构造方法的id.都这样写.
	jmethodID consID = (*env)->GetMethodID(env,objectClass, "<init>", "()V");

	//获取类中每一个变量的定义
	jfieldID begYear = (*env)->GetFieldID(env,objectClass, "begYear", "S");
	jfieldID begMonth = (*env)->GetFieldID(env,objectClass, "begMonth", "S");
	jfieldID begDay = (*env)->GetFieldID(env,objectClass, "begDay", "S");

	//创建一个jobject对象.
	jobject myReturn = (*env)->NewObject(env,objectClass, consID);

	//给每一个实例的变量赋值
	(*env)->SetIntField(env,myReturn, begYear, beg.wYear);
	(*env)->SetIntField(env,myReturn, begMonth, beg.wMonth);
	(*env)->SetIntField(env,myReturn, begDay, beg.wDay);

	return myReturn;*/
}

JNIEXPORT void JNICALL Java_com_howell_ecameraap_VedioList_closeFileList
(JNIEnv *env, jclass cls,int file_list_handle){
	int ret = hwnet_close_file_list(file_list_handle);
	__android_log_print(ANDROID_LOG_INFO, "jni", "close list ret %d\n",ret);
}

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_playbackPause
(JNIEnv *env, jclass cls,jboolean bPause){
	int ret = hwplay_pause(res->play_handle,bPause);
	__android_log_print(ANDROID_LOG_INFO, "jni", "pause %d",ret);
}

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_ptzTurnLeft
(JNIEnv *env, jobject obj,jint slot){

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

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_ptzTurnRight
(JNIEnv *env, jobject obj,jint slot){

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

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_ptzTurnUp
(JNIEnv *env, jobject obj,jint slot){
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

JNIEXPORT void JNICALL Java_com_howell_ecameraap_HWCameraActivity_ptzTurnDown
(JNIEnv *env, jobject obj,jint slot){

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
