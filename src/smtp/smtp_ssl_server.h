
#ifndef __SMTP_SSL_SERVER_H__
#define __SMTP_SSL_SERVER_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

extern bool SMTP_ssl_server_check(const char* server_domain);

#ifdef __cplusplus
};
#endif
#endif //__SMTP_SSL_SERVER_H__

