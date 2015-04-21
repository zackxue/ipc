
#include "socket_tcp.h"
#include <assert.h>

static int tcp_close(struct SOCKET_TCP *const tcp)
{
	return close(tcp->sock);
}

static int tcp_connect(struct SOCKET_TCP *const tcp, const char* serv_addr, in_port_t serv_port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serv_port);
	addr.sin_addr.s_addr = inet_addr(serv_addr);
	return connect(tcp->sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
}

static int tcp_listen(struct SOCKET_TCP *const tcp, in_port_t port, int backlog)
{
	int ret = 0;
	struct sockaddr_in addr;

	// bind
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	ret = bind(tcp->sock, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0){
		return ret;
	}

	// listen
	ret = listen(tcp->sock, backlog);
	if(ret < 0){
		return ret;
	}
	return 0;
}

static int tcp_send(struct SOCKET_TCP *const tcp, const void *msg, size_t len, int flags)
{
	return send(tcp->sock, msg, len, flags);
}

static int tcp_send2(struct SOCKET_TCP *const tcp, const void *msg, size_t len)
{
	int snd_len = len;
	const void *p = msg; // from beginning

	while(snd_len > 0){
		int const snd_size = tcp->send(tcp, p, snd_len, 0);
		if(snd_size < 0){
			if(EINTR == errno){
				return -1;
			}
			if(EAGAIN == errno){
				usleep(1000);
				continue;
			}
			return -1;
		}
		if(snd_size == snd_len){
			return len;
		}

		// proc the left
		snd_len -= snd_size;
		p += snd_size;
	}
	return len - snd_len;
}

static ssize_t tcp_recv(struct SOCKET_TCP *const tcp, void *buf, size_t len, int flags)
{
	return recv(tcp->sock, buf, len, flags);
}

static ssize_t tcp_recv2(struct SOCKET_TCP *const tcp, void *buf, size_t size)
{
	int ret=0;
	int recv_len = 0;
	void *pbuf = buf; // the head of buffer
	while(recv_len < size)	{
		ret = tcp->recv(tcp, pbuf,size - recv_len, 0);
		if(ret <= 0){
			return -1;
		}else{
			pbuf += ret;
			recv_len += ret;
		}
	}
	return recv_len;
}

static ssize_t tcp_peek(struct SOCKET_TCP *const tcp, void *buf, size_t len)
{
	return tcp->recv(tcp, buf, len, MSG_PEEK);
}

static int tcp_send_timeout(struct SOCKET_TCP *const tcp, const void *msg, size_t len, time_t timeout_s)
{
	int ret = 0;
	size_t send_len = 0;
	int send_errno = 0;
	time_t const base_time = time(NULL);

	while(send_len < len){
		if(time(NULL) - base_time > timeout_s){
			// send timeout
			// only send a part of buffer
			break;
		}

		ret = send(tcp->sock, msg + send_len, len - send_len, MSG_DONTWAIT);
		send_errno = errno;
		if(ret < 0){
			if(EAGAIN == send_errno || EINTR == send_errno){
				// need to retry
				continue;
			}
			return ret;
		}
		send_len += ret;
	}
	return send_len;
}


static int tcp_getsockname(struct SOCKET_TCP *const tcp, struct sockaddr *name, socklen_t *namelen)
{
	return getsockname(tcp->sock, name, namelen);
}

static int tcp_getpeername(struct SOCKET_TCP *const tcp, struct sockaddr *name, socklen_t *namelen)
{
	return getpeername(tcp->sock, name, namelen);
}

static int tcp_set_send_timeout(struct SOCKET_TCP *const tcp, time_t timeout_s, suseconds_t timeout_us)
{
	struct timeval tv = { .tv_sec = timeout_s, .tv_usec = timeout_us, };
	return setsockopt(tcp->sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static int tcp_set_recv_timeout(struct SOCKET_TCP *const tcp, time_t timeout_s, suseconds_t timeout_us)
{
	struct timeval tv = { .tv_sec = timeout_s, .tv_usec = timeout_us, };
	return setsockopt(tcp->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static int tcp_set_reuser_addr(struct SOCKET_TCP *const tcp, bool flag)
{
	int reuse_on = flag ? 1 : 0;
	return setsockopt(tcp->sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on));
}

static lpSOCKET_TCP tcp_create_r(int sock, lpSOCKET_TCP result)
{
	
	// create a system socket
	result->sock = sock;
	
	// install the interfaces
	result->close = tcp_close;
	result->connect = tcp_connect;
	result->listen = tcp_listen;
	result->send = tcp_send;
	result->send2 = tcp_send2;
	result->send_timeout = tcp_send_timeout;
	result->recv = tcp_recv;
	result->recv2 = tcp_recv2;
	result->peek = tcp_peek;
	result->getsockname = tcp_getsockname;
	result->getpeername = tcp_getpeername;
	
	// sock option
	result->set_send_timeout = tcp_set_send_timeout;
	result->set_recv_timeout = tcp_set_recv_timeout;
	result->set_reuser_addr = tcp_set_reuser_addr;

	// default to set reuse
	result->set_reuser_addr(result, true);
	
	// success
	return result;
}

lpSOCKET_TCP socket_tcp_r(lpSOCKET_TCP result)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	return tcp_create_r(sock, result);
}

lpSOCKET_TCP socket_tcp2_r(int sock, lpSOCKET_TCP result)
{
	return tcp_create_r(sock, result);
}


