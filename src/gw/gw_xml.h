#ifndef __GW_XML_H__
#define __GW_XML_H__

extern const char *strnchr(const char *str, size_t len, int character);
extern char Char2Num(char ch) ;
extern int parse_path_info(void *_pSession, char *_xml, const int _xml_size);
extern int url_decode(void *_pSession, char* result, const int resultSize);
#endif /*__GW_XML_H__*/
