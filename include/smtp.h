
#ifndef	__SMTP_H__
#define	__SMTP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <semaphore.h>
#include <time.h>

typedef struct SMTP
{
	uint32_t fobidden_zero;
}SMTP_SESSION_t;

extern int SMTP_init(bool msgout);
extern void SMTP_destroy();

extern SMTP_SESSION_t* SMTP_login(const char* server, uint16_t port, const char* usermail, const char* password);
extern void SMTP_logout(SMTP_SESSION_t* smtp);

extern int SMTP_send(SMTP_SESSION_t* const smtp, char* recvmail, char* subject, char* body,
	const char* attach_file, const char* file_name);
extern const char* SMTP_strerror(SMTP_SESSION_t* const smtp);

/////////////////////////////////////////////////////////////////////////////////////
// example by Law
/////////////////////////////////////////////////////////////////////////////////////
//#define _TEST_SMTP
#ifdef _TEST_SMTP
int main(int argc, char** argv)
{
	SMTP_init(true);

	SMTP_SESSION_t* const smtp = SMTP_login("smtp.163.com", 25, "juandevice@163.com", "juantech");

	SMTP_send(smtp, "lawishere@yeah.net", "EMAIL TESTING", "THIS IS BODY!", "/tmp/ipcam.jpg");

	SMTP_logout(smtp);

	SMTP_destroy();
	return 0;
}
#endif

#ifdef __cplusplus
};
#endif
#endif //__SMTP_H__

