
#include "web_server.h"
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "http_server.h"
#include "app_debug.h"

static lpHTTP_SERV _http_serv = NULL;

int WEBS_init(const char* resource_dir)
{
	if(!_http_serv){
		_http_serv = HTTP_SERV_init();
		if(NULL != resource_dir){
			_http_serv->set_resource_dir(_http_serv, resource_dir);
		}
		return 0;
	}
	return -1;
}

void WEBS_destroy()
{
	if(_http_serv){
		HTTP_SERV_destroy(_http_serv);
		_http_serv = NULL;
	}
}

SPOOK_SESSION_PROBE_t WEBS_probe(const void* msg, size_t msg_sz)
{
	return HTTP_SERV_probe(msg, msg_sz) ? SPOOK_PROBE_MATCH : SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t WEBS_loop(bool *trigger, int sock, time_t *read_pts)
{
	if(_http_serv){
		_http_serv->session_loop(_http_serv, trigger, sock);
	}
	return SPOOK_LOOP_SUCCESS;
}


