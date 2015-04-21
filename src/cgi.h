

#ifndef __CGI_H__
#define __CGI_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "httpd.h"

extern int CGI_moo(HTTPD_SESSION_t* session);

extern int CGI_system_upgrade(HTTPD_SESSION_t* session);
extern int CGI_system_upgrade_get_rate(HTTPD_SESSION_t* session);

extern int CGI_flv_live_view(HTTPD_SESSION_t* session);

extern int CGI_whoami(HTTPD_SESSION_t* session);
extern int CGI_shell(HTTPD_SESSION_t* session);

extern int CGI_snapshot(HTTPD_SESSION_t* session);
extern int CGI_mjpeg(HTTPD_SESSION_t* session);
extern int CGI_mjpeg_html(HTTPD_SESSION_t* session);

extern int CGI_focus_measure(HTTPD_SESSION_t *session);



// server: the smtp server address, default following the user mail address
// port: the smtp server address opening port, default 25
// usermail: which is the sendor
// password: sendor password
// target: which is the receiver
// subject: the email subject, defualt 'no subject'
// content: the email text content, defualt '<null>'
// snapshot : yes/no, whether to get an image from device, default no
// vin : the snapshot camera channel of device
// size : the image size of snapshot
extern int CGI_send_email(HTTPD_SESSION_t* session);

extern int CGI_sdk_reg_rw(HTTPD_SESSION_t* session);

extern int CGI_today_snapshot(HTTPD_SESSION_t* session);

extern int CGI_onvif_process(HTTPD_SESSION_t *session);

#ifdef __cplusplus
};
#endif
#endif //__CGI_H__

