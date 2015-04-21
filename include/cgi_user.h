
#ifndef __CGI_USER_H__
#define __CGI_USER_H__

#include "httpd.h"

#define CGI_USER_VERSION "1.0"

extern int CGI_user_list(HTTPD_SESSION_t* http_session);

//
// ?content=
// <user>
//  <add_user name="" password="" admin="" premit_live="" premit_setting="" premit_playback="" />
// </user>
//
extern int CGI_add_user(HTTPD_SESSION_t* http_session);

//
// ?content=
// <user>
//  <del_user name="" />
// </user>
//
extern int CGI_del_user(HTTPD_SESSION_t* http_session);

//
// ?content=
// <user>
//  <edit_user name="" admin="" premit_live="" premit_setting="" premit_playback="" />
// </user>
//
extern int CGI_edit_user(HTTPD_SESSION_t* http_session);

//
// ?content=
// <user>
//  <set_pass old_pass="" new_pass="" />
// </user>
//
extern int CGI_user_set_password(HTTPD_SESSION_t* http_session);


#endif //__CGI_USER_H__

