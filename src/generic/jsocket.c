
#include "jsocket.h"

ssize_t jsock_send(int fd, const void* msg, size_t len)
{
	ssize_t snd_size;
	size_t snd_len = len;
	const void *p = msg; // from beginning

	while(1)
	{
		snd_size = send(fd, p, snd_len, 0);
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

		if((size_t)snd_size == snd_len){
			return len;
		}

		// proc the left
		snd_len -= snd_size;
		p += snd_size;
	}

	return snd_size;
} 

