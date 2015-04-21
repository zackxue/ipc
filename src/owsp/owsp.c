/*
 * protocol_owsp.c
 *
 *  Created on: 2012-6-13
 *      Author: ted
 */
//#include <string.h>
//#include <sys/types.h>

#include "spook/owsp.h"
#include "owsp_def.h"
#include "owsp_debug.h"
#include "owsp_imp.h"

#include "socket_rw.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "generic.h"

typedef struct OwspSession
{
	uint32_t* trigger;
	int is_init;
	int is_login;
	int sock;

	lpMEDIABUF_USER user; // the mediabuf user

#define  OWSP_SESSION_RECV_BUF_SZ (2 * 1024)
	uint8_t recv_buf[OWSP_SESSION_RECV_BUF_SZ];
	ssize_t recv_sz;
	
}OwspSession_t;


#define true	1
#define false	0

#define HEART_TIMEOUT (5)
#define MAX_BUF_SIZE (1024*20/*128*/)

#ifndef MAX_CAM_CH
#define MAX_CAM_CH 32
#endif
int bChannelOpen[MAX_CAM_CH];


static OwspSession_t* owsp_create_session(uint32_t* trigger, int sock)
{
	OwspSession_t* const session = (OwspSession_t*)(calloc(sizeof(OwspSession_t), 1));

	session->trigger = trigger;
	session->is_init = 1;
	session->is_login = 0;
	session->sock = sock;
	//session->recv_sz = recv(session->sock, session->recv_buf, ARRAY_SIZE(session->recv_buf), 0);
	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, sock, (void *)session->recv_buf, ARRAY_SIZE(session->recv_buf), 1);
	SOCKETRW_read(&rw_ctx);
	session->recv_sz = rw_ctx.actual_len;
	OWSP_TRACE("SIZE:%d  result:%d", session->recv_sz, rw_ctx.result);
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS){
		free(session);
		return NULL;
	}
	// the media buf user
	session->user = NULL;
	return session;
}

static void owsp_destroy_session(OwspSession_t* const session)
{
	if(session->user){
		MEDIABUF_detach(session->user);
		session->user = NULL;
	}
	free(session);
}

void reset_sendState()
{
}


static int prosess_sendpack(OwspSession_t* session, OWSP_CONX owsp)
{
	int ret = 0;
	int nSendFrameCnt = 0;
	uint32_t static mediabuf_in_speed = 0;
	if (!session->is_init && session->user == NULL) {
		int mediabuf_ch = MEDIABUF_lookup_byname("qvga.264");
		if(mediabuf_ch >= 0){
			mediabuf_in_speed = MEDIABUF_in_speed(mediabuf_ch);
			if(mediabuf_in_speed > 0){
				session->user = MEDIABUF_attach(mediabuf_ch);
//				OWSP_TRACE("session->user = %08x", session->user);
			}
		}else{
			mediabuf_ch = MEDIABUF_lookup_byname("360p.264");
			if(mediabuf_ch >= 0){
				mediabuf_in_speed = MEDIABUF_in_speed(mediabuf_ch);
				if(mediabuf_in_speed > 0){
					session->user = MEDIABUF_attach(mediabuf_ch);
	//				OWSP_TRACE("session->user = %08x", session->user);
				}
			}
		}
	}
	if(session->is_login){
		if(session->user){
			while(*session->trigger)
			{
				if(0 == MEDIABUF_out_lock(session->user)){
					void* send_ptr = 0;
					ssize_t send_sz = 0;
					if(0 == MEDIABUF_out(session->user, &send_ptr, NULL, &send_sz)){
						const lpSDK_ENC_BUF_ATTR const attr = (lpSDK_ENC_BUF_ATTR*)send_ptr;
						TLV_V_StreamDataFormat StrmInfo;
						if(!session->is_init){
							session->is_init = 1;
							if(OWSP_SendStreamDataFormat(owsp, StrmInfo) == -1){
								session->is_login = 0;
								return -1;
							}
						}
						ret = OWSP_SendFrame(owsp,
									send_ptr + sizeof(stSDK_ENC_BUF_ATTR),
									send_sz - sizeof(stSDK_ENC_BUF_ATTR),
									attr->h264.keyframe);
						nSendFrameCnt++;
						MEDIABUF_out_unlock(session->user);
						if(ret  == -1){
							session->is_login = 0;
							return -1;
						}
						//new_connect = 0;
					}else{
						MEDIABUF_out_unlock(session->user);
//						OWSP_TRACE("nSendFrameCnt = %02d---%02d---%08x", nSendFrameCnt, owsp.Chnl,session->user);
						break;
					}

				}
				//usleep(mediabuf_in_speed);
			}

		}
	}
	return 0;
}

static int process_pack(OwspSession_t* session, OWSP_CONX *owsp,unsigned char* buf, int buf_sz)
{
	int  iCount = 0;

    int  tmpSize = buf_sz;
    char*tmpBuff = buf;
    int  pSize   = 0;     //Every Time Tlv Segment to Process
    int  iRet    = 0;

    TLV_HEADER tmpTlvHdr;
//	int send_frame_ready = 0;
//	int new_connect = 0;


	while(tmpSize > (int)sizeof(tmpTlvHdr)) {
        memcpy(&tmpTlvHdr, tmpBuff, sizeof(tmpTlvHdr));

		OWSP_TRACE("TLV Sequece %d, Type %d, TLV Len %d\n", iCount, tmpTlvHdr.tlv_type, tmpTlvHdr.tlv_len);

        pSize = tmpTlvHdr.tlv_len;

		tmpSize -= sizeof(tmpTlvHdr);
		tmpBuff += sizeof(tmpTlvHdr);

        iRet = !0;

        if(pSize > tmpSize) {
            OWSP_TRACE("OWSP -----> Wrong Size\n");
            return -1;
        }

		printf("owsp->");
		owsp->Sock = session->sock;
		int connect = 1;
		printf("pnum:");
		owsp->pNum = &connect;
		printf("%d\n", *(owsp->pNum));
        //Check Available Connection
        if(!(owsp->pNum) || *(owsp->pNum) >= MAX_CAM_CH) {
            char BuffRet[OWSP_RESPONSE_LEN];
            TLV_V_LoginResponse LoginRet;
            LoginRet.result = _RESPONSECODE_MAX_USER_ERROR;
            OWSP_SendPacket(owsp, TLV_T_LOGIN_ANSWER,
                BuffRet, (char *)&LoginRet, sizeof(LoginRet));
            return -1;
        }//*/

		switch(tmpTlvHdr.tlv_type) {
			case TLV_T_VERSION_INFO_REQUEST:
                iRet = OWSP_Ver_Info_ReqProc(owsp, tmpBuff, pSize);
				OWSP_TRACE("Version info proc ret : %d\n", iRet);
				break;
			case TLV_T_LOGIN_REQUEST:
                iRet = OWSP_Usr_Login_ReqProc(owsp, tmpBuff, pSize);
				OWSP_TRACE("Login info proc ret : %d\n", iRet);
				if(iRet) {
                	bzero(bChannelOpen,sizeof(bChannelOpen));
                    bChannelOpen[owsp->Chnl] = true;

					session->is_login = 1;
					session->is_init = 0;
                }
				break;
			case TLV_T_CONTROL_REQUEST:
                iRet = OWSP_Ptz_Ctrl_ReqProc(owsp, tmpBuff, pSize);
				break;
			case TLV_T_CHANNLE_REQUEST:
                iRet = OWSP_Chn_Switch_ReqProc(owsp, tmpBuff, pSize);
                if(iRet) {
                    bzero(bChannelOpen,sizeof(bChannelOpen));
                    bChannelOpen[owsp->Chnl] = true;
                }
				break;
			case TLV_T_SUSPEND_CHANNLE_REQUEST:
                iRet = OWSP_Chn_Suspend_ReqProc(owsp, tmpBuff, pSize);
				break;
			case TLV_T_PHONE_INFO_REQUEST:
				iRet = OWSP_Mob_Info_ReqProc(owsp, tmpBuff, pSize);
				break;
			case TLV_T_SENDDATA_REQUEST:
			case TLV_T_SUSPENDSENDDATA_REQUEST:
			case TLV_T_DEVICE_FORCE_EXIT:
			case TLV_T_RECORD_REQUEST:
			case TLV_T_DEVICE_SETTING_REQUEST:
			case TLV_T_DEVICE_RESET:
			case TLV_T_VALIDATE_REQUEST:
			case TLV_T_DVS_INFO_REQUEST:
			default:
                return !0;
				break;
		}

        if(0 < iRet) {
            ++ iCount;
		}
		tmpSize -= pSize;
        tmpBuff += pSize;
	}

    return iCount;
}

static int process_pack_pre(OwspSession_t* session, OWSP_CONX *owsp)
//	MEDIABUF_USER_t* user, int socket_fd, unsigned char* _buf, int _buf_size)
{
	char* Owsp_Buff = session->recv_buf;
	int   Owsp_Size = session->recv_sz;
	OwspPacketHeader  tmpPckHdr;

	OWSP_TRACE("Processing Data...OWSP BUFFER SIZE: %d\n", Owsp_Size);

	if(Owsp_Size > (int)sizeof(tmpPckHdr)) {  //One Time Parser
		memcpy(&tmpPckHdr, Owsp_Buff, sizeof(OwspPacketHeader));

		//Read & Convert Packet Length
		unsigned int  tmpSize = 0;
		unsigned char tmpBuff[sizeof(tmpSize)];  //Avoid Alignment Problem
		memcpy(tmpBuff, Owsp_Buff, sizeof(tmpBuff));  //Get OWSP Packet Length
		tmpSize = ntohl(*(unsigned int*)tmpBuff); //Change To Host Byte Order

		tmpPckHdr.packet_length = tmpSize;

		OWSP_TRACE("Processing Data...OWSP POCKET SIZE: %u\n", (unsigned int)tmpPckHdr.packet_length);
		OWSP_TRACE("Processing Data...OWSP POCKET ORDR: %u\n", (unsigned int)tmpPckHdr.packet_seq);

		if(tmpSize <= sizeof(tmpPckHdr.packet_seq)) {
			return -1;
		}
		tmpSize   -= sizeof(tmpPckHdr.packet_seq);

		Owsp_Size -= sizeof(tmpPckHdr);  //Move Buffer pointer
		Owsp_Buff += sizeof(tmpPckHdr);

		if(Owsp_Size >= (int)tmpSize) {
			OWSP_TRACE("Processing Data...OWSP SUCCESS\n");
			//return ProtlRecvEnd_OWSP(pSockInf, Owsp_Buff, tmpSize);
			//continue parse
			process_pack(session, owsp, Owsp_Buff, Owsp_Size);
			//while (tmpSize > (int)sizeof(tmpTlvHdr)) {
			//}
			return 0;
		}
		else { //FIXME: Break to Parser Only Once;
			OWSP_TRACE("Processing Data...OWSP FAILED\n");
			return -1;
		}

		Owsp_Size -= tmpSize;
		Owsp_Buff += tmpSize;
	}
	else {
		OWSP_TRACE("size error.\n");
	}
	return 0;
}

SPOOK_SESSION_PROBE_t OWSP_probe(const void* msg, ssize_t msg_sz)
{
	if (msg_sz < 4 )
		return SPOOK_PROBE_UNDETERMIN;

	char *_msg = msg;
	int ret = (_msg[0] == 0x00) && (_msg[1] == 0x00) && (_msg[2] == 0x00) && (_msg[3] == 0x48);

	if (ret != 1) {
		return SPOOK_PROBE_MISMATCH;
	}
	else if (ret == 1) {
		return SPOOK_PROBE_MATCH;
	}
}

SPOOK_SESSION_LOOP_t OWSP_loop(bool* trigger, int sock, time_t *read_pts)
{
	int ret = -1;
//	MEDIABUF_USER_t* user = MEDIABUF_attach(1);

	OwspSession_t* const session = owsp_create_session(trigger, sock);
	OWSP_CONX owsp;
	ret = process_pack_pre(session, &owsp);

	struct timeval timeout;
	fd_set read_set;
	int select_ret;
	
	signal(SIGPIPE, SIG_IGN);
	
	while(*session->trigger)
	{
		/*if(time(NULL) - *read_pts> HEART_TIMEOUT)
		{
			close(sock);
			*trigger= 0;
			printf("timeout exit\n");
			continue;
		}*/

		timeout.tv_sec = 0;
		timeout.tv_usec = 10*1000;//read command every 10ms

		FD_ZERO(&read_set);
		FD_SET(sock, &read_set);
		select_ret = select(sock + 1, &read_set, NULL, NULL, &timeout);


		if (select_ret < 0)
		{
			*trigger = 0;
			printf("error stop\n");
			continue;
		}
		else if(select_ret == 0)
		{
//			printf("running...\n");
		}
		else
		{
			printf("select read\n");
			//char buf[MAX_BUF_SIZE];

			SOCKET_RW_CTX rw_ctx;
			SOCKETRW_rwinit(&rw_ctx, sock, (void *)session->recv_buf, sizeof(session->recv_buf), 1);
			SOCKETRW_read(&rw_ctx);
			session->recv_sz = rw_ctx.actual_len;
			OWSP_TRACE("socket_fd:%d    select read result: %d\n", sock, rw_ctx.result);
			if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
			{
				*trigger = 0;
				printf("read error stop\n");
				reset_sendState();
			}
			else
			{
				*read_pts= time(NULL);
//				buf[read_ret] = 0;
				session->recv_buf[session->recv_sz] = 0;
				process_pack_pre(session, &owsp);
			}
		}
		if((ret = prosess_sendpack(session, owsp)) == -1){
			OWSP_TRACE("Send packet error!!!");
			break;
		}
	}

	// destroy
	OWSP_TRACE("Destroy---%08x---%08x", session->sock, session->user);
	owsp_destroy_session(session);

	return SPOOK_LOOP_SUCCESS;
}


