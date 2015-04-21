/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa_soup.c
 * Describle: parse the soup protocol data
 * History:  
 * Last modified: 2013-03-28 17:36
 =============================================================*/

#include "union_protocol.h"
#include "ezxml.h"
#include "rudpa_soup.h"
#include "rudpa_debug.h"

#include "sdk/sdk_api.h"
#include "media_buf.h"


#define STREAM0 "720p.264"
#define STREAM1 "360p.264"
#define STREAM2 "qvga.264"

static char *PackStreamInfo(SoupData *thiz)
{
	char *s;
	ezxml_t soup = ezxml_new("SOUP");
	ezxml_set_attr(soup,"version","1.0");

	ezxml_t settings = ezxml_add_child(soup,"settings",0);
	ezxml_set_attr(settings,"method","read");
	ezxml_set_attr(settings,"ticket",thiz->soup_ticket);

	ezxml_t vin = ezxml_add_child(settings,thiz->settings_vin,0);
	
	if(0 == 0)
	{
		ezxml_t stream0 = ezxml_add_child(vin,"stream0",0);
		ezxml_set_attr(stream0,"name",STREAM0);
		ezxml_set_attr(stream0,"size","1280x720");
		ezxml_set_attr(stream0,"x1","yes");
		ezxml_set_attr(stream0,"x2","yes");
		ezxml_set_attr(stream0,"x4","yes");
	}
	if(0 == 0)
	{
		ezxml_t stream1 = ezxml_add_child(vin,"stream1",1);
		ezxml_set_attr(stream1,"name",STREAM1);
		ezxml_set_attr(stream1,"size","640x360");
		ezxml_set_attr(stream1,"x1","yes");
		ezxml_set_attr(stream1,"x2","yes");
		ezxml_set_attr(stream1,"x4","yes");
	}
	if(0 == 0)
	{
		ezxml_t stream2 = ezxml_add_child(vin,"stream2",2);
		ezxml_set_attr(stream2,"name",STREAM2);
		ezxml_set_attr(stream2,"size","320x240");
		ezxml_set_attr(stream2,"x1","yes");
		ezxml_set_attr(stream2,"x2","yes");
		ezxml_set_attr(stream2,"x4","yes");
	}
	
	s = ezxml_toxml(soup);
	char *stream_info = strdup(s);
	free(s);
	s = NULL;
	return stream_info;	
}


char * PackSoupPkt(SoupData *thiz)
{
	char buf[1024];
	char *pSoupPkt = NULL;
	switch(thiz->soup_cmd)
	{
		case SoupCmdPtz:
			{
				sprintf(buf,"<SOUP version=\"1.0\"><ptz error=\"%d\" ticket=\"%s\"/></SOUP>",
							ES_SUCCESS,thiz->soup_ticket);
				pSoupPkt = strdup(buf);				
			}
			break;
		case SoupCmdSeekStream:
			{
				/*get the ipc offer streaminfo*/
				pSoupPkt = PackStreamInfo( thiz);
			}
			break;
		case SoupCmdAuth:
		{
				sprintf(buf,"<SOUP version=\"1.0\"><auth usr=\"%s\" psw=\"%s\" error=\"%d\"/></SOUP>",
						thiz->auth_usr,thiz->auth_psw,thiz->soup_error);
				pSoupPkt = strdup(buf);
		}
			break;
		case SoupCmdDevinfo:
		{
			sprintf(buf,"<SOUP version=\"1.0\"><devinfo camcnt=\"%d\"/></SOUP>",thiz->sd_camcnt);
			pSoupPkt = strdup(buf);
		}
			break;
		
	}
	return pSoupPkt;
}

static int GetFrameType(const lpSDK_ENC_BUF_ATTR attr)
{
	// video
#define AVENC_BUF_DATA_H264 (0x00000000)
#define AVENC_BUF_DATA_JPEG (0x00000001)
	// audio
#define AVENC_BUF_DATA_PCM (0x80000000)
#define AVENC_BUF_DATA_G711A (0x80000001)
#define AVENC_BUF_DATA_G711U (0x80000002)

	if( (attr->type  & AVENC_BUF_DATA_PCM) == AVENC_BUF_DATA_PCM)
	{
		/*audio*/
		return 0;
	}
	else if(0 != attr->h264.keyframe)
	{
		/*i frame*/
		return (0x1);
	}
	else
	{
		return (0x2);
	}	
}

int PackSoupHead(void *data_buf,const lpSDK_ENC_BUF_ATTR attr)
{
	
	SoupFrameHead *soupHead = (SoupFrameHead*)data_buf;
	soupHead->magic = 0x534f55ff;/*SOU.*/
	soupHead->version = 0x10000000;
	soupHead->frametype = GetFrameType(attr);
	soupHead->framesize = attr->data_sz;
	soupHead->pts = attr->time_us;
	soupHead->externsize = 0;
	if(soupHead->frametype > 0)
	{
		soupHead->_U.v.width = attr->h264.width;
		soupHead->_U.v.height = attr->h264.height;
		memcpy(&soupHead->_U.v.enc,"H264",strlen("Hxxx"));
	}
	else
	{
		soupHead->_U.a.samplerate = attr->g711a.sample_rate;
		soupHead->_U.a.samplewidth = attr->g711a.sample_width;
		memcpy(&soupHead->_U.a.enc,"G711",strlen("Gxxx"));	
	}
	return 0;
}

static int  GetVin(const char *str_vin)
{
	return atoi(str_vin+3);
}

static int GetStreamNo(const char *str_streamNo)
{
	return atoi(str_streamNo+6);
}
static void GetVinStream(ezxml_t xml, uint32_t*vin,uint32_t *stream_no)
{
	*vin = GetVin(ezxml_attr(xml,"ch"));
	*stream_no = GetStreamNo(ezxml_attr(xml,"stream"));
}

char* GetStreamName(int stream_No)
{
	char *p;
	switch(stream_No)
	{
		case 0:
			p = strdup(STREAM0);
			break;
		case 1:
			p = strdup(STREAM1);
			break;
		case 2:
			p = strdup(STREAM2);
			break;
		default:
			p = NULL;
			break;				
	}
	return p;
}


int  SoupDataProc(char *pData,int nDatasize,SoupData*soupData)
{	

	if(0 != strncmp(pData,"<SOUP",strlen("<SOUP")))
	{
		DBG("NOT SOUP Pkt\n");
		return ES_HEADERROR;
	}

	/*parse the soup data*/
	ezxml_t soup_xml = ezxml_parse_str(pData ,strlen(pData));
	ezxml_t soup_cmd = soup_xml->child;
	if(NULL == soup_cmd)
	{
		DBG("NULL Soup string\n");
		return -1;
	}
	
	soupData->soup_ticket = ezxml_attr(soup_cmd,"ticket") 
	                                     ? strdup(ezxml_attr(soup_cmd,"ticket")):strdup("");
	if(0 == strcmp("ptz",ezxml_name(soup_cmd)))
	{		
		soupData->soup_cmd = SoupCmdPtz;
		soupData->ptz_chl = ezxml_attr(soup_cmd,"chl") ? strdup(ezxml_attr(soup_cmd,"chl")):strdup("");
		/*left param to read*/
	}
	else if(0 == strcmp("settings",ezxml_name(soup_cmd)))
	{
		soupData->soup_cmd =SoupCmdSeekStream;
		soupData->settings_method = ezxml_attr(soup_cmd,"method") 
		                                          ? strdup(ezxml_attr(soup_cmd,"method")): strdup("");
		ezxml_t vin_xml = soup_cmd->child;
		soupData->settings_vin = ezxml_name(vin_xml) ? strdup(ezxml_name(vin_xml)):strdup("");
	}
	else if(0 == strcmp("streamreq",ezxml_name(soup_cmd)))
	{
		soupData->soup_cmd =SoupCmdReqStream;
		GetVinStream(soup_cmd,&soupData->streamreq_ch,&soupData->streamreq_stream_No);
		soupData->streamreq_opt = ezxml_attr(soup_cmd,"opt") ? strdup(ezxml_attr(soup_cmd,"opt")):strdup("");
	}
	else if(0 == strcmp("auth",ezxml_name(soup_cmd)))
	{
		soupData->soup_cmd = SoupCmdAuth;
		soupData->auth_usr = ezxml_attr(soup_cmd,"usr") ?strdup(ezxml_attr(soup_cmd,"usr")):strdup("");
		soupData->auth_psw = ezxml_attr(soup_cmd,"psw")? strdup(ezxml_attr(soup_cmd,"psw")):strdup("");
	}
	else if(0 == strcmp("devinfo",ezxml_name(soup_cmd)))
	{
		soupData->soup_cmd = SoupCmdDevinfo;
	}
	else
	{
		_RUDPA_ERROR("SOUP data Parse error\n");
		soupData->soup_cmd = -1;
	}
	_RUDPA_DEBUG("soup data parse Done\n");
	return 0;
}


Soup* CreateNewSoup(void*data)
{
	UnionProtocol * up = (UnionProtocol*)data;
	Soup * pSoup = (Soup*)calloc(sizeof(Soup),1);
	if(NULL == pSoup)
	{
		_RUDPA_ERROR("calloc mem error\n");
		return NULL;
	}
	up->up_soup = pSoup;
	pSoup->DataProc = SoupDataProc;
	pSoup->PackPkt = PackSoupPkt;
	pSoup->PackHead = PackSoupHead;
	return pSoup;
}



