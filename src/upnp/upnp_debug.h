
#ifndef __UPNP_DEBUG_H__
#define __UPNP_DEBUG_H__

#define UPNP_DEBUG (0)
#if UPNP_DEBUG
#define UPNP_SYNTAX "1;33"
#define UPNP_TRACE(fmt, arg...) \
	do{\
		const char* bname = strdupa(basename(__FILE__));\
		printf("\033["UPNP_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define UPNP_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = strdupa(basename(__FILE__));\
			printf("\033["UPNP_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define UPNP_TRACE(fmt...)
#define UPNP_ASSERT(exp, fmt...)
#endif

#endif //__UPNP_DEBUG_H__

