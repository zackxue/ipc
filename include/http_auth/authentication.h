#ifndef __AUTHENTICATION_H__
#define __AUTHENTICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_RET_OK		(0)
#define AUTH_RET_FAIL	(-1)


#define HTTP_AUTH_REALM		"www.dvr163.com"

#define HTTP_AUTH_BASIC		(0)
#define HTTP_AUTH_DIGEST	(1) // not support for rtsp server yet,???,but is ok when in rtsp client
#define HTTP_AUTH_DEFAULT_TYPE		HTTP_AUTH_DIGEST

enum{
	HTTP_AUTH_ALGORITHEM_MD5 = 0, // only support md5
	//HTTP_AUTH_ALGORITHEM_SHA,
};

#define HTTP_AUTH_DEFAULT_ALGORITHM	HTTP_AUTH_ALGORITHEM_MD5


typedef struct _authentication
{
	int type;
	// login user and password
	char user[32];
	char pwd[32];

	int algorithm;
	char realm[128];
	char url[128];
	char nonce[128];
	char method[32];
	char responce[128];
}Authentication_t;

// use for client
extern int HTTP_AUTH_client_init(struct _authentication **auth,
	char *www_authenticate/*the value in the domain of <WWW-Authenticate> not contain CRLF*/);
extern int HTTP_AUTH_setup(struct _authentication *auth,
	char *username,char *password,
	char *url,char *method, /* if use digest ,must given these two parameters,else ignore these*/
	char *out,int out_size);
// use for server
extern int HTTP_AUTH_server_init(struct _authentication **auth,int type);
extern int HTTP_AUTH_chanllenge(struct _authentication *auth,char *out,int out_size);
extern int HTTP_AUTH_validate(struct _authentication *auth,
	char *authorization,/* the value in the domain of <Authorization>,not contain CRLF */
	char *method);
//
extern int HTTP_AUTH_destroy(struct _authentication *auth);

#ifdef __cplusplus
}
#endif
#endif
