#include "hw_std.h"
#include <net_sdk.h>
#include "stream_buf.h"

stream_buf* g_buf = NULL;
char* g_stream;
char* g_out = NULL;

void sigstop(int signal)
{
    exit(0);
}

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
				nalus[nalu_count].nalu = nalu_buf + 4;
				nalus[nalu_count].timestamp = timestamp;
				nalus[nalu_count].size = nalu_len - 4;
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
		nalus[nalu_count].nalu = nalu_buf + 4;
		nalus[nalu_count].timestamp = timestamp;
		nalus[nalu_count].size = nalu_len - 4;
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
            memcpy(dst,nalu_info[i].nalu,nalu_info[i].size);
            dst += nalu_info[i].size;
            dst_len += nalu_info[i].size;
        }
    }

    *ret_len = dst_len;
}


void stream_fun(LIVE_STREAM_HANDLE handle,int stream_type,const char* buf,int len,long userdata)
{
    printf("on stream come#stream_type=%d,len=%d\n",stream_type,len);
}

typedef struct {
	long len;
	long type; //0-bbp frame,1-i frame,2-audio
	unsigned long long time_stamp;
	long tag;
	long sys_time;
	//long reserve[1];
}stream_head;

void file_fun(FILE_STREAM_HANDLE handle,const char* buf,int len,long userdata)
{
   if(stream_buf_input_data(g_buf,buf,len) < 0)
   {
       printf(">>>>>>>1\n");
       return;
   }
   else
   {
       while(1)
       {
           stream_head head;
           if(stream_buf_copy_data(g_buf,(char*)&head,sizeof(head)) < 0)
           {
               break;
           }

           if(stream_buf_get_data(g_buf,g_stream,head.len) < 0)
           {
               break;
           }

           struct tm* t = gmtime(&head.sys_time);
           printf("%04d-%02d-%02d %02d:%02d:%02d\n",t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);

           int out_len = 0;
           parse_test(g_stream + sizeof(head),head.len - sizeof(head),g_out,1024 * 1024,&out_len,head.time_stamp / 1000);
           printf("out_len = %d\n",out_len);
       }
   }
}

int main(int argc,char* argv[])
{
    if(argc < 2)
    {
        printf("use: %s ip\n",argv[0]);
        return -1;
    }

    signal(SIGINT,sigstop);
	signal(SIGQUIT,sigstop);
	signal(SIGTERM,sigstop);

    /*首先登录到设备上,获取user handle*/
    USER_HANDLE uh = hwnet_login(argv[1],5198,"admin","12345");
    if(uh < 0)
    {
        printf("login failed\n");
        return -1;
    }

    //LIVE_STREAM_HANDLE sh = hwnet_get_live_stream(uh,0,0,0,stream_fun,0);
    //hwnet_save_to_jpg(uh,0,1,75,"/tmp/1.jpg");
#if 0
    Pagination page_info;
    memset(&page_info,0,sizeof(page_info));
    page_info.page_size = 50;
    page_info.page_no = 0;
#endif

    g_buf = stream_buf_open(1024 * 1024);
    g_stream = (char*)malloc(1024 * 1024);
    g_out = (char*)malloc(1024 * 1024);

    SYSTEMTIME beg;
    memset(&beg,0,sizeof(beg));
    SYSTEMTIME end;
    beg.wYear = 2015;
    beg.wMonth = 6;
    beg.wDay = 15;
    beg.wHour = 10;
    beg.wMinute= 50;
    beg.wSecond= 57;
    end = beg;
    end.wHour = 10;
    end.wMinute= 51;
    end.wSecond= 6;

    printf("get files start\n");
    file_stream_t file_info;
    FILE_LIST_HANDLE fh =  hwnet_get_file_stream(uh,0,beg,end,file_fun,0,&file_info);
    assert(fh > -1);
    printf(">>>>total_size=%d\n",file_info.len);

    printf("press ctrl c to exit\n");
    while(1)
    {
        sleep(1);
    }

    return 0;
}
