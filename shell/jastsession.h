#ifndef __JASTSESSION_H__
#define __JASTSESSION_H__

#ifdef __cplusplus
extern "C" {
#endif	

typedef struct _SessionElem
{
	union
	{
		struct{
			char ip[20];
			int fd;
			void *context;
		};
		struct{
			int entries;
		};
	};
}SessionElem_t;

typedef struct _session
{	
	SessionElem_t data;
	struct _session *next;
}JastSession_t;


int JAST_session_init();
int JAST_session_destroy();
JastSession_t *JAST_session_add(char *ip_dst,int sock,void *context);
int JAST_session_del(char *ip_dst);
JastSession_t* JAST_session_find(char *ip_dst);
int JAST_session_entries();
JastSession_t* JAST_session_get(int index);
int JAST_session_dump();

#ifdef __cplusplus
}
#endif
#endif

