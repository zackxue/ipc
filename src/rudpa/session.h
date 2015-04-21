#ifndef __SESSION_H__
#define __SESSION_H__


int session_init();

SESSION_t *get_session(int sid);
SESSION_t *session_begin(LISTEN_t * thiz,UDP_PKT_H_t *uph,u_int32_t len,int sock_fd,u_int32_t ip,u_int16_t port);
int session_quit(SESSION_t *s);
int _session_close(UDP_PKT_H_t *uph);

#endif
