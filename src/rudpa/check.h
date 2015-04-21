#ifndef __CHECK_H_
#define __CHECK_H_

int check_init();
int check_pkt(SESSION_t *s,char *data,int datalen);
void check_send_pkt_timeout(SESSION_t *s,time_t t);
int check_ack_pkt(SESSION_t *s,char *data,int datalen);

#endif
