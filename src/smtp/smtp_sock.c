

ssize_t SMTP_SOCK_send(int sock, const void* buf, ssize_t size, int timeout_s)
{
	int ret = 0;
	int ret_errno = 0;

	time_t const begin_time = time(NULL);
	time_t current_time = begin_time;

	uint8_t* buf_ptr = buf;
	ssize_t send_len = 0;

	while(1)
	{
		assert(send_len <= size);//, "send_len=%d, size=%d", send_len, size);
		if(size == send_len){
			// send completed
			return send_len;
		}
		if(current_time - begin_time >= timeout_s){
			// send timeout
			return send_len;
		}

//		TRACE_DEBUG("waitting_size=%d", waitting_size);
		ret = send(sock, _ctx->buf + send_len, size - send_len, 0);
		ret_errno = errno;
		current_time = time(NULL);
		if(ret < 0){
			if(EAGAIN == ret_errno || EINTR == ret_errno){
				// retry
				continue;
			}
			printf("send failed errno = %d\r\n", ret_errno);
			return -1;
		}else if(ret >= 0){
			send_len += ret;
			if(0 == ret){
				printf("ret is zero, errno_cpy=%d\r\n", ret_errno);
			}
		}
	}
	return -1;
}

