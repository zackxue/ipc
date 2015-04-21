
#ifndef __UART_H__
#define __UART_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

typedef struct UART
{
	int (*set_baud)(struct UART* const thiz, int baud);	// 2400 ...
	int (*get_baud)(struct UART* const thiz);
	int (*set_databit)(struct UART* const thiz, int databit);	// 5 6 7 8
	int (*get_databit)(struct UART* const thiz);
	int (*set_stopbit)(struct UART* const thiz, int stopbit);	// 1 2
	int (*get_stopbit)(struct UART* const thiz);
#define UART_PARITY_NONE (0)
#define UART_PARITY_ODD (1)
#define UART_PARITY_EVEN (2)
	int (*set_parity)(struct UART* const thiz, int parity);	// n o e
	int (*get_parity)(struct UART* const thiz);
	ssize_t (*write)(struct UART* const thiz, const void* bytes, ssize_t len);
	ssize_t (*read)(struct UART* const thiz, void* ret_bytes, ssize_t len);
}UART_t;

extern UART_t* UART_open(const char* device);
extern void UART_close(UART_t* uart);

#endif	//__UART_H__

