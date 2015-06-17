#ifndef stream_buf_include_h
#define stream_buf_include_h



typedef struct stream_buf {
	char* buf_beg;
	char* buf_end;
	char* data_beg;
	int data_len;

}stream_buf;

enum
{
    STREAM_BUF_SUCCESS = 0,
    STREAM_BUF_NOT_ENOUGH_BUF = -1,
    STREAM_BUF_INVALID_PARAM = -2,
};

/**
 * 说明:    申请一个环回buf
 * 参数:    
 *          capability:buf的大小 
 * 返回:    NULL:申请失败，否则成功
 */
stream_buf* stream_buf_open(int capability);


/**
 * 说明:    释放一个环回buf
 * 参数:    
 *          sb:stream buf指针 
 * 返回:    0:成功 其他失败 
 */
int stream_buf_close(stream_buf* sb);


/**
 * 说明:    获取环回buf的有效数据长度
 * 参数:    
 *          sb:stream buf指针 
 *          data_len:返回长度
 * 返回:    0:成功 其他失败 
 */
int stream_buf_get_data_len(stream_buf* sb,int* data_len);


/**
 * 说明:    获取环回buf的总容量
 * 参数:    
 *          sb:stream buf指针 
 *          cap_len:返回长度,该长度等于stream_buf_open()的长度
 * 返回:    0:成功 其他失败 
 */
int stream_buf_get_capability(stream_buf* sb,int* cap_len);


/**
 * 说明:    获取环回buf的可用容量
 * 参数:    
 *          sb:stream buf指针 
 *          reamain_len:返回长度,该长度等于总容量-数据长度
 * 返回:    0:成功 其他失败 
 */
int stream_buf_get_remain_len(stream_buf* sb,int* remain_len);


/**
 * 说明:    将数据放入环回buf 
 * 参数:    
 *          sb:stream buf指针 
 *          buf:数据
 *          len:数据长度
 * 返回:    0:成功 其他失败(一般为可用容量小于放入的数据长度)
 */
int stream_buf_input_data(stream_buf* sb,const char* buf,int len);


/**
 * 说明:    移除环回buf中的数据
 * 参数:    
 *          sb:stream buf指针 
 *          out:移除数据至保存的目标buf(上层确保out由足够的容量来容纳数据)
 *          len:移除数据长度
 * 返回:    0:成功 其他失败
 */
int stream_buf_get_data(stream_buf* sb,char* out,int len);


/**
 * 说明:    复制环回buf中的数据
 * 参数:    
 *          sb:stream buf指针 
 *          out:复制数据至保存的目标buf(上层确保out由足够的容量来容纳数据)
 *          len:复制数据长度
 * 返回:    0:成功 其他失败
 */
int stream_buf_copy_data(stream_buf* sb,char* out,int len);



stream_buf* stream_buf_open(int capability)
{
    stream_buf* sb = (stream_buf*)malloc(sizeof(stream_buf));
    if(sb == NULL)
    {
        return NULL;
    }

    sb->buf_beg = (char*)malloc(capability);
    if(sb->buf_beg == NULL)
    {
        free(sb);
        return NULL;
    }

    sb->data_beg = sb->buf_beg;
    sb->data_len = 0;
    sb->buf_end = sb->buf_beg + capability;

    return sb;
}

int stream_buf_close(stream_buf* sb)
{
    free(sb->buf_beg);
    free(sb);

    return STREAM_BUF_SUCCESS;
}

int stream_buf_get_data_len(stream_buf* sb,int* data_len)
{
	__android_log_print(ANDROID_LOG_INFO, "jni", "11111");
    if(data_len == NULL)
    {
        return STREAM_BUF_INVALID_PARAM;
    }
    __android_log_print(ANDROID_LOG_INFO, "jni", "22222");
    *data_len = sb->data_len;
    __android_log_print(ANDROID_LOG_INFO, "jni", "33333");
    return STREAM_BUF_SUCCESS;
}

int stream_buf_get_capability(stream_buf* sb,int* cap_len)
{
    if(cap_len == NULL)
    {
        return STREAM_BUF_INVALID_PARAM;
    }

    *cap_len = sb->buf_end - sb->buf_beg;

    return STREAM_BUF_SUCCESS;
}

int stream_buf_get_remain_len(stream_buf* sb,int* remain_len)
{
    if(remain_len == NULL)
    {
        return STREAM_BUF_INVALID_PARAM;
    }

    int buf_len = sb->buf_end - sb->buf_beg;

    *remain_len = buf_len - sb->data_len;

    return STREAM_BUF_SUCCESS;
}

int stream_buf_input_data(stream_buf* sb,const char* buf,int len)
{
    int remain_len = 0;
    int ret = stream_buf_get_remain_len(sb,&remain_len);
    assert(ret == STREAM_BUF_SUCCESS);

    if(remain_len < len)
    {
        return STREAM_BUF_NOT_ENOUGH_BUF;
    }

    char* data_end = sb->data_beg + sb->data_len ;
    if(data_end >= sb->buf_end)
    {
        data_end = sb->buf_beg + (data_end - sb->buf_end);
    }

    if(data_end + len > sb->buf_end)
    {
        int left =  sb->buf_end - data_end;
        memcpy(data_end,buf,left);

        memcpy(sb->buf_beg,(char*)&buf[left],len - left);			
    }else{
        memcpy(data_end,buf,len);			
    }

    sb->data_len += len;
    return STREAM_BUF_SUCCESS;
}

int stream_buf_get_data(stream_buf* sb,char* out,int len)
{
    if(sb->data_len < len)
    {
        return STREAM_BUF_NOT_ENOUGH_BUF;
    }

    if(sb->data_beg + len > sb->buf_end)
    {
        int left = sb->buf_end - sb->data_beg;
        memcpy(out,sb->data_beg,left);

        sb->data_beg = sb->buf_beg;
        memcpy((char*)&out[left],sb->data_beg,len - left);
        sb->data_beg += (len - left);
    }
    else
    {
        memcpy(out,sb->data_beg,len);
        sb->data_beg += len;
    }

    sb->data_len -= len;

    return STREAM_BUF_SUCCESS;
}

int stream_buf_copy_data(stream_buf* sb,char* out,int len)
{
    if(sb->data_len < len)
    {
        return STREAM_BUF_NOT_ENOUGH_BUF;
    }

    if(sb->data_beg + len > sb->buf_end)
    {
        int left = sb->buf_end - sb->data_beg;
        memcpy(out,sb->data_beg,left);

        memcpy((char*)&out[left],sb->buf_beg,len - left);			
    }else{
        memcpy(out,sb->data_beg,len);			
    }		

    return STREAM_BUF_SUCCESS;
}




#endif
