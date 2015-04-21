
#include "smtp_debug.h"
#include "smtp.h"
#include "smtp_eml.h"
#include "smtp_ssl_server.h"
#include "base64.h"
#include "openssl/err.h"
#include "openssl/ssl.h"

typedef struct SMTP_SESSION_BODY
{
	// info
	char server[64];
	uint16_t server_port;
	char usermail[64];
	char password[32];
	char recvmail[64];
	// socket
	int sock;
	// ssl about
	bool ssl_use;
	SSL_CTX* ssl_context;
	SSL* ssl_sock;
	
	char str_error[512];
}SMTP_SESSION_BODY_t;

const char *SmtpServerTypeString[] = {
	"smtp.gmail.com", // 25
	"smtp.163.com", // 25
	"smtp.qq.com", // 25
	"smtp.yahoo.com", // 25
};

#define SMTP_PERROR(_thiz,_s,_str_error) \
	do{\
		strncpy(_thiz->str_error, _str_error, sizeof(_thiz->str_error));\
		SMTP_TRACE("%s %s", _s, _thiz->str_error);\
	}while(0)

static int smtp_echo(SMTP_SESSION_BODY_t* const thiz, char* buf, ssize_t buf_limit)
{
	int ret = 0;
	ret = send(thiz->sock, buf, strlen(buf), 0);
	if(ret < 0){
		perror("send");
	}
	// FIXME:
	assert(strlen(buf) == ret);


	ret = recv(thiz->sock, buf, buf_limit, 0);
	if(!(ret > 0)){
		fprintf(stderr, "recv err = %d %s\r\n", errno, strerror(errno));
	}
	buf[ret] = '\0';
	return atoi(buf);
}

static int smtp_echo_ssl(SMTP_SESSION_BODY_t* const thiz, char* buf, ssize_t buf_limit)
{
	int ret = 0;
	if(!thiz->ssl_use){
		return smtp_echo(thiz, buf, buf_limit);
	}
	ret = SSL_write(thiz->ssl_sock, buf, strlen(buf));
	if(ret < 0){
		SMTP_TRACE("%s", ERR_reason_error_string(ERR_get_error()));
	}
	assert(strlen(buf) == ret);
	ret = SSL_read(thiz->ssl_sock, buf, buf_limit);
	if(ret < 0){
		SMTP_TRACE("%s", ERR_reason_error_string(ERR_get_error()));
	}
	assert(ret > 0);
	buf[ret] = '\0';
	return atoi(buf);
}


static SMTP_SESSION_t* smtp_create(const char* server, uint16_t port, const char* usermail, const char* password)
{
	int ret = 0;
	SMTP_SESSION_t* const smtp = calloc(sizeof(SMTP_SESSION_t) + sizeof(SMTP_SESSION_BODY_t), 1);
	SMTP_SESSION_BODY_t* const thiz = (SMTP_SESSION_BODY_t*)(&smtp[1]);
	struct timeval const timeout = {10,0}; 

	// info
	strncpy(thiz->server, server, sizeof(thiz->server) - 1);
	thiz->server_port = 25; // fixed;
	strncpy(thiz->usermail, usermail, sizeof(thiz->usermail) - 1);
	strncpy(thiz->password, password, sizeof(thiz->password) - 1);
	memset(thiz->recvmail, 0, sizeof(thiz->recvmail));

	// socket
	thiz->sock = socket(AF_INET, SOCK_STREAM, 0);
	assert(thiz->sock > 0);
	ret = setsockopt(thiz->sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	assert(0 == ret);
	ret = setsockopt(thiz->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	assert(0 == ret);

	// ssl
	if(SMTP_ssl_server_check(thiz->server)){
		thiz->ssl_use = true;
		thiz->ssl_context = SSL_CTX_new(SSLv23_client_method());
		assert(thiz->ssl_context);
		thiz->ssl_sock = SSL_new(thiz->ssl_context);
		assert(thiz->ssl_sock);
	}else{
		thiz->ssl_use = false;
		thiz->ssl_context = NULL;
		thiz->ssl_sock = NULL;
	}

	memset(thiz->str_error, 0, sizeof(thiz->str_error));
	return smtp;
}

static void smtp_release(SMTP_SESSION_t* smtp)
{
	if(smtp){
		SMTP_SESSION_BODY_t* const thiz = (SMTP_SESSION_BODY_t*)(smtp + 1);
		if(thiz->ssl_use){
			SSL_shutdown(thiz->ssl_sock);
			SSL_free(thiz->ssl_sock);
			SSL_CTX_free(thiz->ssl_context);
		}
		close(thiz->sock);
		free(smtp);
    }
}

static int smtp_login_connect(SMTP_SESSION_BODY_t* const thiz)
{
	char buf[1024];
	int ret = 0;
	
	struct sockaddr_in dest_addr;
	struct hostent*	host_entry = 0;
	struct in_addr* host_addr;

	SMTP_TRACE("try to connect SMTP server: %s", thiz->server);

	// get host ip
	host_entry = gethostbyname2(thiz->server, AF_INET);
	if(!host_entry){
		SMTP_TRACE("dns error!");
		perror("gethostbyname");
		return -1;
	}
	host_addr = (struct in_addr*)(host_entry->h_addr_list[0]);
	
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(thiz->server_port);
	dest_addr.sin_addr.s_addr = host_addr[0].s_addr;

	// get an error ip address
	SMTP_TRACE("server ip: %s", inet_ntoa(dest_addr.sin_addr));
	//dest_addr.sin_addr.s_addr = inet_addr("220.181.12.15");

	ret = connect(thiz->sock, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr));
	if(ret < 0){
		perror("connect");
		return -1;
	}
	ret = recv(thiz->sock, buf, sizeof(buf), 0);
	assert(ret >= 0);

	if(ret > 0){
		buf[ret] = '\0';
		ret = atoi(buf);
		if(220 == ret){
			return 0;
		}else{
			SMTP_PERROR(thiz, "CONNECT", buf);
		}
	}
	return -1;
}

static int smtp_login_setup_ssl(SMTP_SESSION_BODY_t* const thiz)
{
	if(thiz->ssl_use){
		int ret = 0;
		char buf[1024];
		ret = smtp_echo(thiz, "STARTTLS\r\n", sizeof(buf));
		if(220 != ret){
			SMTP_PERROR(thiz, "STARTTLS", buf);
			return -1;
		}
		// setup ssl socket
		SSL_set_fd(thiz->ssl_sock, thiz->sock);
		ret = SSL_connect(thiz->ssl_sock);
		if(ret < 0){
			SMTP_TRACE("%s", ERR_reason_error_string(ERR_get_error()));
		}
		assert(0 == ret);
	}
	return 0;
}

static int smtp_login_auth(SMTP_SESSION_BODY_t* const thiz)
{
	int ret = 0;
	char buf[1024];
	strcpy(buf, "AUTH LOGIN\r\n");
	ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
	if(334 == ret){
		char* usermail_base64 = alloca(strlen(thiz->usermail) * 2);
		char* password_base64 = alloca(strlen(thiz->password) * 2);

		base64_encode(thiz->usermail, usermail_base64, strlen(thiz->usermail));
		base64_encode(thiz->password, password_base64, strlen(thiz->password));
		
//		SMTP_TRACE("usermail: %s|%s", thiz->usermail, usermail_base64);
//		SMTP_TRACE("password: %s|%s", thiz->password, password_base64);

		snprintf(buf, sizeof(buf), "%s\r\n", usermail_base64);
		ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
		if(334 == ret){
			snprintf(buf, sizeof(buf), "%s\r\n", password_base64);
			ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
			if(235 == ret){
//				SMTP_TRACE("AUTH success");
				return 0;
			}else{
				SMTP_PERROR(thiz, "AUTH PASSWORD", buf);
			}
		}else{
			SMTP_PERROR(thiz, "AUTH USERMAIL", buf);
		}
	}else{
		SMTP_PERROR(thiz, "AUTH", buf);
	}
	return -1;
}

static int smtp_login(SMTP_SESSION_t* const smtp)
{
	SMTP_SESSION_BODY_t* const thiz = (SMTP_SESSION_BODY_t*)(smtp + 1);
	int ret = 0;
	char buf[1024];

	ret = smtp_login_connect(thiz);
	if(ret < 0){
		return -1;
	}

	sprintf(buf, "EHLO %s\r\n", thiz->server);
	ret = smtp_echo(thiz, buf, sizeof(buf));
	if(250 == ret){
		ret = smtp_login_setup_ssl(thiz);
		assert(0 == ret);

		ret = smtp_login_auth(thiz);
		if(0 != ret){
			return -1;
		}
	}else{
		SMTP_PERROR(thiz, "EHLO", buf);
		return -1;
	}
	
	SMTP_TRACE("user %s login success", thiz->usermail);
	return 0;
}

SMTP_SESSION_t* SMTP_login(const char* server, uint16_t port, const char* usermail, const char* password)
{
	SMTP_SESSION_t* const smtp = smtp_create(server, port, usermail, password);
	if(0 == smtp_login(smtp)){
		return smtp;
	}
	smtp_release(smtp);
	return NULL;
}

void SMTP_logout(SMTP_SESSION_t* smtp)
{
	if(smtp){
		SMTP_SESSION_BODY_t* const thiz = (SMTP_SESSION_BODY_t*)(smtp + 1);
		int ret = 0;
		char buf[1024];

		strcpy(buf, "QUIT \r\n");
		ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
		if(221 != ret){
			SMTP_PERROR(thiz, "QUIT", buf);
		}
		smtp_release(smtp);
    }
}

static void smtp_set_sockopt(int sock, ssize_t mail_size, int timeout_s)
{
	int ret = 0;
//	int const snd_bps = 32 * 1024;
	ssize_t const snd_buf_size = (mail_size + 1023) & ~1023;
//	struct timeval snd_timeout;

	SMTP_TRACE("set sock send buffer = %d", snd_buf_size);
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&snd_buf_size, sizeof(snd_buf_size));
	assert(0 == ret);

	//snd_timeout.tv_sec = timeout_s;
//	snd_timeout.tv_sec = mail_size * 8 / snd_bps;
//	snd_timeout.tv_usec = 0;
//	SMTP_TRACE("set sock send timeout = %d", (int)snd_timeout.tv_sec);
//	ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&snd_timeout, sizeof(snd_timeout));
//	assert(0 == ret);

}

int SMTP_send(SMTP_SESSION_t* const smtp, char* recvmail, char* subject, char* body,
	const char* attach_file, const char* file_name)
{
	int ret = 0;
	char buf[1024];
	SMTP_SESSION_BODY_t* const thiz = (SMTP_SESSION_BODY_t*)(smtp + 1);
	ssize_t mail_size = 0;
	char* mail_buf = NULL;

	if(!recvmail && !strlen(thiz->recvmail)){
		strcpy(thiz->recvmail, thiz->usermail); // send to itself
	}else{
		strcpy(thiz->recvmail, recvmail);
	}
	if(!subject){
		subject = "null";
	}
	if(!body){
		body = "null";
	}

	// calc mail size
	mail_size = SMTP_eml_estimate(subject, body, attach_file);
	SMTP_TRACE("eml estimated size = %d", mail_size);
	// make the eml file
	mail_buf = calloc(mail_size, 1);
	SMTP_eml_make(mail_buf, mail_size, subject, body, attach_file, file_name);
	
	SMTP_TRACE("start to send email size=%d!", strlen(mail_buf));

	ret = snprintf(buf, sizeof(buf), "MAIL FROM:<%s> \r\n", thiz->usermail);
	assert(ret < sizeof(buf));
	ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
	if(250 == ret){
		ret = snprintf(buf, sizeof(buf), "RCPT TO:<%s> \r\n", thiz->recvmail);
		assert(ret < sizeof(buf));
		ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
		if(250 == ret){
			strcpy(buf, "DATA\r\n");
			ret = smtp_echo_ssl(thiz, buf, sizeof(buf));
			if(354 == ret){
				// set socket send buffer size & timeout
				smtp_set_sockopt(thiz->sock, strlen(mail_buf), 0);
				ret = smtp_echo_ssl(thiz, mail_buf, mail_size);
				if(250 == ret){
					SMTP_TRACE("succeed to send data email !");
					free(mail_buf);
					return 0;
				}else{
					SMTP_PERROR(thiz, "DATA EML", buf);
				}
			}else{
				SMTP_PERROR(thiz, "DATA", buf);
			}
		}else{
			SMTP_PERROR(thiz, "RCPT", buf);
		}
	}else{
		SMTP_PERROR(thiz, "MAIL", buf);
	}

	free(mail_buf);
	return -1;
}

const char* SMTP_strerror(SMTP_SESSION_t* const smtp)
{
	SMTP_SESSION_BODY_t* const thiz = (SMTP_SESSION_BODY_t*)(smtp + 1);
	return thiz->str_error;
}


int SMTP_init(bool msgout)
{
	// init ssl
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	return 0;
}

void SMTP_destroy()
{
	return;
}

