
#ifndef __HTTPD_DEBUG_H__
#define __HTTPD_DEBUG_H__

#define HTTPD_DEBUG (1)
#if HTTPD_DEBUG
#define HTTPD_SYNTAX "1;31"
#define HTTPD_TRACE(fmt, arg...) \
	do{\
		const char* bname = strdupa(basename(__FILE__));\
		printf("\033["HTTPD_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define HTTPD_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = strdupa(basename(__FILE__));\
			printf("\033["HTTPD_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define HTTPD_TRACE(fmt...)
#define HTTPD_ASSERT(exp, fmt, arg...)
#endif


#endif //__HTTPD_DEBUG_H__

