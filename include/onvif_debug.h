#ifndef ONVIF_DEBUG_H
#define ONVIF_DEBUG_H

#define FONT_COLOR_BLACK	"30"
#define FONT_COLOR_RED		"31"
#define FONT_COLOR_GREEN	"32"
#define FONT_COLOR_YELLOW	"33"
#define FONT_COLOR_BLUE		"34"
#define FONT_COLOR_WHITE	"37"

#define FONT_BKG_BLACK		"40"
#define FONT_BKG_RED		"41"
#define FONT_BKG_GREEN		"42"
#define FONT_BKG_YELLOW		"43"
#define FONT_BKG_BLUE		"44"
#define FONT_BKG_WHITE		"47"

#define CLOSE_ALL_ATTR		"0m"
#define SET_HEIGHT			"1m"
#define SET_UNDERLINE		"4m"
#define SET_FLICKER			"5m"
#define CLEAR_SCREEN		"2J"


#define ONVIF (1)
//#define PRINT_DEFAULT_ATTR "31"
#if ONVIF
#define ONVIFTRACE(fmt...) \
	do{\
		printf("\033["FONT_BKG_BLACK";"FONT_COLOR_WHITE"m");\
		printf(fmt);\
		printf("\033["CLOSE_ALL_ATTR"\r\n");\
	}while(0);
#define ONVIFDEBUG(fmt...) \
    do{\
        printf("\033["FONT_BKG_BLACK";"FONT_COLOR_YELLOW"m");\
        printf("[%s:%d]FUNCTION:%s\n", basename(__FILE__), __LINE__, __FUNCTION__);\
        printf("\033["FONT_BKG_BLACK";"FONT_COLOR_WHITE"m");\
        printf(fmt);\
		printf("\033["CLOSE_ALL_ATTR"\r\n");\
    }while(0);
#else
#define ONVIFTRACE(fmt...)
#define ONVIFDEBUG(fmt...)

#endif

#endif
