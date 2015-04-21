
#include "base64.h"
#include "generic.h"
#include "httpd_debug.h"
#include "httpd.h"

#define HTTPD_AUTH_USER_BACKLOG (16)

typedef struct HTTPD_AUTH_USER
{
	bool registered;
	char username[32];
	char password[32];
	// for quick auth
	char base64_auth[96];

}HTTPD_AUTH_USER_t;

typedef struct HTTPD_AUTH
{
	int user_counter;
	HTTPD_AUTH_USER_t user[HTTPD_AUTH_USER_BACKLOG];
}HTTPD_AUTH_t;
static HTTPD_AUTH_t _http_auth =
{
	.user_counter = 0,
};
static HTTPD_AUTH_t* _p_http_auth = &_http_auth;

void HTTPD_auth_dump()
{
	int i = 0;
	HTTPD_TRACE("HTTP users %d:", _p_http_auth->user_counter);
	for(i = 0; i < ARRAY_ITEM(_p_http_auth->user); ++i){
		HTTPD_AUTH_USER_t* const user = _p_http_auth->user + i;
		if(user->registered){
			printf("\t%d. \"%s\" \"%s\" \"%s\"\r\n", i, user->username, user->password, user->base64_auth);
		}
	}
}

int HTTPD_auth_user_cnt()
{
	return _p_http_auth->user_counter;
}

int HTTPD_auth_del(const char* username)
{
	int i = 0;
	for(i = 0; i < ARRAY_ITEM(_p_http_auth->user); ++i){
		HTTPD_AUTH_USER_t* const user = _p_http_auth->user + i;
		// looking for who is registered
		if(user->registered){
			if(STR_CASE_THE_SAME(username, user->username)){
				HTTPD_TRACE("Found %s @ %d", user->username, i);
				user->registered = false;
				--_p_http_auth->user_counter;
				return 0;
			}
		}
	}
	return -1;
}

int HTTPD_auth_add(const char* username, char* password, bool override)
{
	int i = 0;
	int ret = 0;
	if(strlen(username) >= 32 || strlen(password) >= 32){
		// invalid username / password
		return -1;
	}

	// if override user, delete the possible 
	if(override){
		HTTPD_auth_del(username);
	}

	if(_p_http_auth->user_counter < HTTPD_AUTH_USER_BACKLOG){
		for(i = 0; i < ARRAY_ITEM(_p_http_auth->user); ++i){
			HTTPD_AUTH_USER_t* const user = _p_http_auth->user + i;
			if(!user->registered){
				char base64_auth[ARRAY_ITEM(user->base64_auth)];
				++_p_http_auth->user_counter;
				user->registered = true;
				// mark the username & its password
				strcpy(user->username, username);
				strcpy(user->password, password);
				// pre base64
				ret = snprintf(base64_auth, ARRAY_ITEM(base64_auth),
						"%s:%s",
						user->username, user->password);
				base64_encode(base64_auth, user->base64_auth, strlen(base64_auth));
				HTTPD_TRACE("Add: %s %s @ %d", base64_auth, user->base64_auth, i);
				return 0;
			}
		}
	}
	return -1;
}

void HTTPD_auth_clear()
{
	_p_http_auth->user_counter = 0;
	ARRAY_ZERO(_p_http_auth->user);
}

bool HTTPD_auth_access(const char* auth)
{
	if(0 == _p_http_auth->user_counter){
		// no auth users
		// must be accessable
		return true;
	}else{
		int i = 0;
		const char* base64_sym = "Basic ";

		if(0 == strncasecmp(auth, base64_sym, strlen(base64_sym))){
			// use base 64 to auth access
			const char* base64_auth = auth + strlen(base64_sym);

			for(i = 0; i < ARRAY_ITEM(_p_http_auth->user); ++i){
				if(STR_CASE_THE_SAME(_p_http_auth->user[i].base64_auth, base64_auth)){
					HTTPD_TRACE("Found user \"%s\"", _p_http_auth->user[i].username);
					return true;
				}
			}
		}
	}
	return false;
}


