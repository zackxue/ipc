
#include "socket_rw.h"

//int SOCKET_readn(int sock, void* _buf, ssize_t sz, int timeout_s)
//{
//	assert(sz > 0);
//
//	time_t begin_time = time(NULL);
//	time_t curr_time = begin_time;
//	int currsz = 0;
//	int recv_ret;
//	int errno_cpy;
//
//	while(1)
//	{
//		if(sz == currsz)
//		{
//			break;
//		}
//		if(curr_time - begin_time > timeout_s)
//		{
//			break;
//		}
//
//		recv_ret = recv(sock, _buf + currsz, sz - currsz, 0);
//		errno_cpy = errno;
//		curr_time = time(NULL);
//		if(recv_ret == -1)
//		{
//			if(errno_cpy == EAGAIN || errno_cpy == EINTR)
//			{
//				continue; //need retry
//			}
//
//			printf("recv failed errno_cpy=%d", errno_cpy);
//			return -1; //error, return
//		}
//		else if(recv_ret == 0)
//		{
//			return currsz; //client disconnect
//		}
//		else if(recv_ret > 0)
//		{
//			currsz += recv_ret; //read some bytes
//		}
//	}
//	return currsz;// currsz < sz poll_wait
//}
//
//int SOCKET_read(int sock, void* _buf, ssize_t sz, int timeout_s)
//{
//	assert(sz > 0);
//
//	time_t begin_time = time(NULL);
//	time_t curr_time = begin_time;
//	int recv_ret;
//	int errno_cpy;
//
//	while(curr_time - begin_time < timeout_s)
//	{
//		recv_ret = recv(sock, _buf, sz, 0);
//		errno_cpy = errno;
//		curr_time = time(NULL);
//		if(recv_ret == -1)
//		{
//			if(errno_cpy == EAGAIN || errno_cpy == EINTR)
//			{
//				continue; //need retry
//			}
//
//			printf("recv failed errno_cpy=%d", errno_cpy);
//			return -1; //error, return
//		}
//		else if(recv_ret == 0)
//		{
//			return 0; //client disconnect
//		}
//		else if(recv_ret > 0)
//		{
//			//XXX yqw-
//			//printf("recv in mps_proc:%s\n", _buf);
//			return recv_ret; //read some bytes
//		}
//	}
//	return -2;//poll_wait
//}
//
//int SOCKET_writen(int sock, void* _buf, ssize_t sz, int _timeout_sec)
//{
//	signal(SIGPIPE, SIG_IGN);
//
//	time_t begin_time = time(NULL);
//	time_t curr_time = begin_time;
//	unsigned char* waitting_buf = _buf;
//	int waitting_size = sz;
//	int send_ret;
//	int errno_cpy;
//	while(waitting_size > 0)
//	{
//		if(curr_time - begin_time > _timeout_sec)
//		{
//			return sz - waitting_size;
//		}
//
////		printf("waitting_size=%d", waitting_size);
//		send_ret = send(sock, waitting_buf, waitting_size, 0);
//		errno_cpy = errno;
//		curr_time = time(NULL);
//		if(send_ret == -1)
//		{
//			strerror(errno_cpy);
//			printf("send_ret=-1, errno=%d", errno_cpy);
//			if(errno_cpy == EPIPE || errno_cpy == ECONNRESET)
//			{
//				return -1;//socket error
//			}
//			else if(errno_cpy == EAGAIN || errno_cpy == EINTR)
//			{
//				continue;//need retry
//			}
//		}
//		else if(send_ret >= 0)
//		{
//			waitting_buf += send_ret;
//			waitting_size -= send_ret;
//			if(send_ret == 0)
//			{
//				printf("send_ret is zero, errno_cpy=%d", errno_cpy);
//			}
//		}
//	}
//	return sz - waitting_size;//poll_wait or finish
//}




void SOCKETRW_rwinit(SOCKET_RW_CTX* _ctx, int _socketfd, void* _buf, int _request_len, int _timeout_sec)
{
	memset(_ctx, 0, sizeof(SOCKET_RW_CTX));
	_ctx->socketfd = _socketfd;
	_ctx->buf = _buf;
	_ctx->request_len = _request_len;
	_ctx->timeout_sec = _timeout_sec;
}

void SOCKETRW_read(SOCKET_RW_CTX* _ctx)
{
	_ctx->result = SOCKET_RW_RESULT_UNKNOWN;
	_ctx->errno_cpy = 0;

	time_t begin_time = time(NULL);
	time_t curr_time = begin_time;

	while(1)
	{
		if(curr_time - begin_time > _ctx->timeout_sec)
		{
			_ctx->result = SOCKET_RW_RESULT_TIMEOUT;
			return;
		}
		_ctx->actual_len = recv(_ctx->socketfd, _ctx->buf, _ctx->request_len, 0);
		_ctx->errno_cpy = errno;
		curr_time = time(NULL);
		if(_ctx->actual_len == -1)
		{
			if(_ctx->errno_cpy == EAGAIN || _ctx->errno_cpy == EINTR)
			{
				continue; //need retry
			}

			printf("recv failed errno_cpy=%d\r\n", _ctx->errno_cpy);
			_ctx->result = SOCKET_RW_RESULT_ERROR;
			return;
		}
		else if(_ctx->actual_len == 0)
		{
			_ctx->result = SOCKET_RW_RESULT_CLIENT_DISCONNECT;
			return;
		}
		else if(_ctx->actual_len > 0)
		{
			_ctx->result = SOCKET_RW_RESULT_SUCCESS;
			return;
		}
	}
	assert(0);
	return;
}

void SOCKETRW_readn(SOCKET_RW_CTX* _ctx)
{
	_ctx->result = SOCKET_RW_RESULT_UNKNOWN;

	time_t begin_time = time(NULL);
	time_t curr_time = begin_time;
	int recv_ret;

	while(1)
	{
		assert(_ctx->actual_len <= _ctx->request_len);//, "_ctx->actual_len=%d, _ctx->request_len=%d", _ctx->actual_len, _ctx->request_len);
		if(_ctx->request_len == _ctx->actual_len)
		{
			_ctx->result = SOCKET_RW_RESULT_SUCCESS;
			return;
		}
		if(curr_time - begin_time > _ctx->timeout_sec)
		{
			_ctx->result = SOCKET_RW_RESULT_TIMEOUT;
			return;
		}

		recv_ret = recv(_ctx->socketfd, _ctx->buf + _ctx->actual_len, _ctx->request_len - _ctx->actual_len, 0);
		_ctx->errno_cpy = errno;
		curr_time = time(NULL);
		if(recv_ret == -1)
		{
			if(_ctx->errno_cpy == EAGAIN || _ctx->errno_cpy == EINTR)
			{
				continue; //need retry
			}

			printf("recv failed errno_cpy=%d\r\n", _ctx->errno_cpy);
			_ctx->result = SOCKET_RW_RESULT_ERROR;
			return;
		}
		else if(recv_ret == 0)
		{
			_ctx->result = SOCKET_RW_RESULT_CLIENT_DISCONNECT;
			return;
		}
		else if(recv_ret > 0)
		{
			_ctx->actual_len += recv_ret;
		}
	}
	assert(0);
	return;
}

void SOCKETRW_write(SOCKET_RW_CTX* _ctx)
{

}

void SOCKETRW_writen(SOCKET_RW_CTX* _ctx)
{
	_ctx->result = SOCKET_RW_RESULT_UNKNOWN;

	time_t begin_time = time(NULL);
	time_t curr_time = begin_time;
//	unsigned char* waitting_buf = _ctx->buf;
//	int waitting_size = _ctx->request_len;
	int send_ret;

	while(1)
	{
		assert(_ctx->actual_len <= _ctx->request_len);//, "_ctx->actual_len=%d, _ctx->request_len=%d", _ctx->actual_len, _ctx->request_len);
		if(_ctx->request_len == _ctx->actual_len)
		{
			_ctx->result = SOCKET_RW_RESULT_SUCCESS;
			return;
		}
		if(curr_time - begin_time > _ctx->timeout_sec)
		{
			_ctx->result = SOCKET_RW_RESULT_TIMEOUT;
			return;
		}

//		TRACE_DEBUG("waitting_size=%d", waitting_size);
		send_ret = send(_ctx->socketfd, _ctx->buf + _ctx->actual_len, _ctx->request_len - _ctx->actual_len, 0);
		_ctx->errno_cpy = errno;
		curr_time = time(NULL);
		if(send_ret == -1)
		{
			if(_ctx->errno_cpy == EAGAIN || _ctx->errno_cpy == EINTR)
			{
				continue;//need retry
			}
			printf("send failed errno_cpy=%d\r\n", _ctx->errno_cpy);
			_ctx->result = SOCKET_RW_RESULT_ERROR;
			return;
		}
		else if(send_ret >= 0)
		{
			_ctx->actual_len += send_ret;
			if(send_ret == 0)
			{
				printf("send_ret is zero, errno_cpy=%d\r\n", _ctx->errno_cpy);
			}
		}
	}
	assert(0);
	return;
}
