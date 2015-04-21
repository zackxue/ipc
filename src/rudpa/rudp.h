#ifndef __RUDP_H_
#define __RUDP_H_

int udp_pkt_parse(LISTEN_t * thiz,char *data,u_int32_t len,int fd,struct sockaddr_in *clientaddr);
int udp_pkt_payload(SESSION_t *s,char *buf,u_int32_t cmd,char * data, u_int32_t len);
void receive_data_free(RECV_RDP_t *r);
void rudp_recv_timeout_check(SESSION_t *s,time_t t);
int  udp_heart_beat(SESSION_t *s);
int udp_session_close(SESSION_t *s);
int udp_session_close_ack(SESSION_t *s);

#endif
