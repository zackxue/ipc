#include "socket_rw.h"
#include "sdk/sdk_api.h"
#include "regRW_def.h"
#include "spook/regRW.h"
#include "generic.h"

#include "sensor.h"
#include "ucode.h"

static RegRWSession_t* regRW_create_session(bool* trigger, int sock)
{
	RegRWSession_t* const session = (RegRWSession_t*)(calloc(sizeof(RegRWSession_t), 1));

	session->trigger = trigger;
	session->is_init = 1;
	session->sock = sock;
	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, sock, (void *)session->recv_buf, ARRAY_SIZE(session->recv_buf), 1);
	SOCKETRW_read(&rw_ctx);
	session->recv_sz = rw_ctx.actual_len;
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS){
		printf("recv failed\r\n");
		free(session);
		return NULL;
	}

	return session;
}

static int regRW_send_read_ret(RegRWSession_t *session, char *sendbuf, int send_size)
{
	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, session->sock, (void *)sendbuf, send_size, 5);
	SOCKETRW_writen(&rw_ctx);
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
	{
		return -1;
	}
	
	return rw_ctx.actual_len;
}

static void regRW_destroy_session(RegRWSession_t* const session)
{
	free(session);
}

static void regRW_parse(RegRWSession_t *session)
{
	char* buff = (char*)session->recv_buf;
	int  size = session->recv_sz;


	PackHead_t* pack_head = (PackHead_t *)buff;
	if(pack_head->uiLength == (size - 8)){	
		switch(pack_head->iPackType){
			/*case REGRW_TYPE_MT9D131:
				{
					Mt9d131reg_t *reg_info = (Mt9d131reg_t *)(buff + sizeof(PackHead_t));
					if(pack_head->iOperation == 1){//write
						SENSOR_spec_reg_write(reg_info->page, reg_info->addr, reg_info->value);
					}
					else if(pack_head->iOperation == 0){//read
						reg_info->value = SENSOR_spec_reg_read(reg_info->page, reg_info->addr);
						regRW_send_read_ret(session, buff, size);
					}
				}
				break;*/
			case REGRW_TYPE_JUANSN:
				{
					Juansn_t *juansn = (Juansn_t *)(buff + sizeof(PackHead_t));;
					printf("%s-%d\r\n", juansn->sn, strlen(juansn->sn));
					UCODE_write(UCODE_SN_MTD, -1, juansn->sn, strlen(juansn->sn));
				}
				break;
			default:
				printf("unknow reg type!\r\n");
				break;
		}
	}
}


SPOOK_SESSION_PROBE_t SENSOR_REGRW_probe(const void* msg, ssize_t msg_sz)
{
	if (msg_sz < 4 )
		return SPOOK_PROBE_UNDETERMIN;

	char *_msg = (char *)msg;

	int ret = (_msg[0] == 0xbb) && (_msg[1] == 0xdd) && (_msg[2] == 0xcc) && (_msg[3] == 0xaa);

	if (ret != 1) {
		return SPOOK_PROBE_MISMATCH;
	}
	else if (ret == 1) {
		return SPOOK_PROBE_MATCH;
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t SENSOR_REGRW_loop(bool* trigger, int sock, time_t *read_pts)
{
	//	MediaBufUser_t* user = MEDIABUF_user_attach(1);
	
		RegRWSession_t* const session = regRW_create_session(trigger, sock);
	
		struct timeval timeout;
		fd_set read_set;
		int select_ret;

		signal(SIGPIPE, SIG_IGN);
		
		while(*session->trigger)
		{	
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
				//printf("select read\n");
				//char buf[MAX_BUF_SIZE];
	
				SOCKET_RW_CTX rw_ctx;
				SOCKETRW_rwinit(&rw_ctx, sock, (void *)session->recv_buf, sizeof(session->recv_buf), 1);
				SOCKETRW_read(&rw_ctx);
				session->recv_sz = rw_ctx.actual_len;
				if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
				{
					*trigger = 0;
					printf("read error stop\n");
				}
				else
				{
					*read_pts= time(NULL);
	//				buf[read_ret] = 0;
					session->recv_buf[session->recv_sz] = 0;
					regRW_parse(session);
				}
			}
		}
	
		// destroy
		regRW_destroy_session(session);
	
		return SPOOK_LOOP_SUCCESS;

}
