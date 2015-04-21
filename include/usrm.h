
/*
 *
 * user manager routines
 * designed by: Frank Law
 *
 */

#ifndef __USRM_H__
#define __USRM_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#define USR_MANGER_VERSION "1.1"
#define USR_MANGER_TMP_FILE "/tmp/usrm.ini"
#define USR_MANGER_USER_HOURSE_BACKLOG (16)

#define USRM_PERMIT_ALL (0xffffffff)
#define USRM_PERMIT_LIVE (1<<0)
#define USRM_PERMIT_SETTING (1<<1)
#define USRM_PERMIT_PLAYBACK (1<<2)

typedef enum USRM_HOW_ABOUT
{
	USRM_GREAT = 0,
	USRM_INVALID, // invalid operate request
	USRM_USERNAME_TOO_LONG,
	USRM_PASSWORD_TOO_LONG,
	USRM_USER_EXIST, // the user has been exist, for add user action
	USRM_USER_NO_EXIT, // no user what you request, for delete / edit action
	USEM_FULL_HOUSE, // the user house is full, for add user action
	USRM_PASSWORD_ERROR, // the user password error, for login / old password double check when set new password
	USRM_ADMIN_ONLY, // only admin could do this operation
	USRM_NOT_PERMIT, // not permit for this user to operate
	USRM_NEVER_LOGIN, // this user never login
	
}USRM_HOW_ABOUT_t;

typedef struct USRM_I_KNOW_U
{
	uint32_t forbidden_zero;
	const char* username;
	const char* password;
	bool is_admin;
	uint32_t permit_flag;

	USRM_HOW_ABOUT_t (*add_user)(struct USRM_I_KNOW_U* const i_m, const char* what_name, const char* what_password, bool is_admin, uint32_t permit_flag);
	USRM_HOW_ABOUT_t (*del_user)(struct USRM_I_KNOW_U* const i_m, const char* what_name);
	USRM_HOW_ABOUT_t (*edit_user)(struct USRM_I_KNOW_U* const i_m, const char* what_name, bool is_admin, uint32_t permit_flag);
	USRM_HOW_ABOUT_t (*set_password)(struct USRM_I_KNOW_U* const i_m, const char* what_old_password, const char* what_new_password);
	USRM_HOW_ABOUT_t (*dump)(struct USRM_I_KNOW_U* const i_m);
	
}USRM_I_KNOW_U_t;

extern int USRM_init(const char* storage, const char* first_username, const char* first_password);
extern void USRM_destroy();

extern USRM_I_KNOW_U_t* USRM_login(const char* who_r_u, const char* what_s_password);
extern void USRM_logout(USRM_I_KNOW_U_t* i_know_u);

extern USRM_HOW_ABOUT_t USRM_add_user(const char* what_name, const char* what_password, bool is_admin, uint32_t permit_flag, bool over_write);
extern USRM_HOW_ABOUT_t USRM_del_user(const char* what_name);
extern USRM_HOW_ABOUT_t USRM_edit_user(const char* what_name, const char * what_password, bool is_admin, uint32_t permit_flag);
extern USRM_HOW_ABOUT_t USRM_check_user(const char* what_name, const char* what_password);
extern USRM_HOW_ABOUT_t USRM_dump(const char* what_name);

extern int USRM_store();

#ifdef __cplusplus
};
#endif

#endif //__USRM_H__

