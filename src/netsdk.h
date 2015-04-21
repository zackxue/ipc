#ifndef NETSDK_H_
#define NETSDK_H_

#include "httpd.h"

typedef struct{
	int max;
	int val;
}RatioValue_t;

void NETSDK_init();
void NETSDK_destroy();
int NETSDK_process_cgi(HTTPD_SESSION_t *session);


#endif

