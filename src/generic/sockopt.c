

int sockopt_set_reuse(int sock, int timeout_s)
{
	int reuse_on = 1;
	if(0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on))){
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int sockopt_set_send_timeout(int sock, int timeout_s)
{
	struct timeval tvout;
    tvout.tv_sec = timeout_s;
    tvout.tv_usec = 0;
	if(0 != setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tvout, sizeof(tvout))){
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int sockopt_set_recv_timeout(int sock, int timeout_s)
{
	struct timeval tvout;
	tvout.tv_sec = timeout_s;
	tvout.tv_usec = 0;
	if(0 != setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tvout, sizeof(tvout))){
		perror("setsockopt");
		return -1;
	}
	return 0;
}

