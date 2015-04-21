#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netstream.h"
#include "vlog.h"

#ifdef NOCROSS
#define TEST_FILE_FPS	25
#define TEST_FILE_INSPEED	(10000000/TEST_FILE_FPS)


int RTSP_STREAM_init(RtspStream_t *s,const char *name) 
{
	FILE *src;
	strcpy(s->name,name);
	s->inspeed= TEST_FILE_INSPEED;
	s->timestamp=0;
	s->data =NULL;
	s->size = 0;

	s->fps = TEST_FILE_FPS;
	src = fopen(name,"rb");
    if (src == NULL){
		VLOG(VLOG_ERROR,"init stream %s failed.",name);
        return RTSP_RET_FAIL;
    }
	s->param = (void *) src;
	VLOG(VLOG_CRIT,"rtmp stream(%s) init success.",name);
	return RTSP_RET_OK;
}

int RTSP_STREAM_next(RtspStream_t *s) 
{	
	static uint32_t base_ts=0;
	FILE *src = (FILE *)s->param;
	char *buffer=NULL;
	Test264Frame_t frame;
    int ret=fread(&frame,sizeof(Test264Frame_t),1,src);
    if (ret != 1) {
		printf("read file failed,ret:%d err:%d\n",ret,GetLastError());
        return RTSP_RET_FAIL;
    } else {
    	if(s->data == NULL){
			buffer = malloc(512*1024);
			if(buffer == NULL){
				printf("malloc for stream buffer failed\n");
				return RTSP_RET_FAIL;
			}
			s->data = buffer;
    	}
		ret=fread(s->data,frame.size,1,src);
		if(ret <= 0)
			return RTSP_RET_FAIL;
    }
    s->size=frame.size;
    s->timestamp=base_ts;
	s->type = RTSP_STREAM_TYPE_VIDEO;
	s->isKeyFrame=frame.isidr;
	s->fps = TEST_FILE_FPS;
	
	base_ts +=1000/TEST_FILE_FPS;

	VLOG(VLOG_DEBUG,"get stream size:%d ts:%d",frame.size,s->timestamp);
	//printf("get stream size:%d ts:%d",frame.size,s->timestamp);
    return RTSP_RET_OK;
}

int RTSP_STREAM_destroy(RtspStream_t *s) 
{
	FILE *src=NULL;
	if(s->data) free(s->data);
	src = (FILE *)s->param;
    if(src) fclose(src);
	return RTSP_RET_OK;
}

int RTSP_STREAM_reset(RtspStream_t *s) 
{
	FILE *src = (FILE *)s->param;
    fclose(src);
    s->param= (void *)fopen(s->name,"r");
    if (s->param == NULL)
        return RTSP_RET_FAIL;
	return RTSP_RET_OK;
}

#else

int RTSP_STREAM_init(RtspStream_t *s,const char *name) 
{
    int mediabuf_ch;
	stMEDIABUF_USER* user = NULL;

    mediabuf_ch = MEDIABUF_lookup_byname(name);
    if (mediabuf_ch < 0) {
		VLOG(VLOG_ERROR,"lookup by name failed,name:%s",name);
        return RTSP_RET_FAIL;
    }
	strcpy(s->name,name);
    user = MEDIABUF_attach(mediabuf_ch);
    if (!user) {
		VLOG(VLOG_ERROR,"mediabuf attach failed,name:%s",name);
        return RTSP_RET_FAIL;
    }
	s->param = (void *)user;
    s->inspeed = MEDIABUF_in_speed(mediabuf_ch);
	s->timestamp=0;
	s->data =NULL;
	s->size = 0;
	s->fps = 0;
	
    MEDIABUF_sync(user);

	VLOG(VLOG_CRIT,"rtmp stream(%s)(inspeed:%d) init success.",name,s->inspeed);
	
	return RTSP_RET_OK;
}


int RTSP_STREAM_next(RtspStream_t *s) 
{
	int ret;
	stMEDIABUF_USER* user =(stMEDIABUF_USER*) s->param;
    int out_success=false;
	
    if (user==NULL) {
        VLOG(VLOG_ERROR,"mediabuf not ready!");
        return -1;
    }
	
    if (0 == MEDIABUF_out_lock(user)) {
		stSDK_ENC_BUF_ATTR* enc_attr = NULL;
		ssize_t enc_size;
        if (0 == MEDIABUF_out(user,(void **)&enc_attr, NULL, &enc_size)) {
			out_success = true;
			s->size = enc_attr->data_sz;
			s->timestamp = (uint32_t)(enc_attr->timestamp_us/ 1000);
			s->data = (void*)(enc_attr + 1);
			s->isKeyFrame = 0;
			if(kSDK_ENC_BUF_DATA_H264 == enc_attr->type){
				s->isKeyFrame = enc_attr->h264.keyframe;
				s->type =  RTSP_STREAM_TYPE_VIDEO;
				s->fps = enc_attr->h264.fps;
			}else if(kSDK_ENC_BUF_DATA_G711A== enc_attr->type){
				s->type = RTSP_STREAM_TYPE_AUDIO;
			}else{
				s->type = RTSP_STREAM_TYPE_NOT_SUPPORT;
			}
        } else {
            //VLOG(VLOG_INFO,"mediabuf get data failed!");
            //usleep(5*1000);
        }
		MEDIABUF_out_unlock(user);

    } else {
        VLOG(VLOG_ERROR,"mediabuf lock failed!");
    }
	
    return true == out_success ? RTSP_RET_OK : RTSP_RET_FAIL;
}

int RTSP_STREAM_destroy(RtspStream_t *s) 
{
	stMEDIABUF_USER* user =(stMEDIABUF_USER*) s->param;
	if(user)
		MEDIABUF_detach(user);

	VLOG(VLOG_CRIT,"stream destroy success");
    return RTSP_RET_OK;
}
int RTSP_STREAM_reset(RtspStream_t *s) 
{
	stMEDIABUF_USER* user =(stMEDIABUF_USER*) s->param;
	MEDIABUF_detach(user);
	RTSP_STREAM_init(s,s->name);
    return RTSP_RET_OK;
}

#endif
