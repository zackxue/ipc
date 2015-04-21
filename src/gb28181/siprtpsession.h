#ifndef __SIPRTPSESSION_H__
#define __SIPRTPSESSION_H__

#ifdef __cplusplus
extern "C" {
#endif	

typedef struct _SipRtpSessionElem
{
	union
	{
		struct{
			char id[128];
			int flag;
			int port;
			void *context;
			void *data;
		};
		struct{
			int entries;
		};
	};
}SipRtpSessionElem_t;

typedef struct _sip_rtp_session
{	
	SipRtpSessionElem_t data;
	struct _sip_rtp_session *next;
}SipRtpSession_t;


int SIPRTP_session_init();
int SIPRTP_session_destroy();
SipRtpSession_t *SIPRTP_session_add(char *id,int port,void *context,void *data);
int SIPRTP_session_set_context(SipRtpSession_t *s,void *context);
int SIPRTP_session_toggle(SipRtpSession_t *s,int flag);
int SIPRTP_session_del(char *id);
SipRtpSession_t* SIPRTP_session_find(char *id);
int SIPRTP_session_entries();
SipRtpSession_t* SIPRTP_session_get(int index);
int SIPRTP_session_dump();

#ifdef __cplusplus
}
#endif
#endif

