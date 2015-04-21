
#include "smtp_ssl_server.h"

static const char* smtp_ssl_server_list[] =
{
	"smtp.live.com",
	"smtp.gmail.com",
};

bool SMTP_ssl_server_check(const char* server_domain)
{
	int i = 0;
	for(i = 0; i < sizeof(smtp_ssl_server_list) / sizeof(smtp_ssl_server_list[0]); ++i){
		if(0 == strncasecmp(server_domain, smtp_ssl_server_list[i], strlen(smtp_ssl_server_list[i]))){
			return true;
		}
	}
	return false;
}

