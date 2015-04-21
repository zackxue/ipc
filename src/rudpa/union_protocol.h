/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		union_protocol.h
 * Describle: this file may contain esee,turn,traversal.etc 
 * History: 
 * Last modified:	2013-03-18 14:49
 ============================================================*/
#ifndef UNION_PROTOCOL_H
#define UNION_PROTOCOL_H

#include "turn.h"
#include "esee.h"
#include "traversal.h"
#include "rudpa_soup.h"
/*the struct may contain more protocol handle*/ 
typedef struct _tagUnionProtocol{
	Esee *up_esee;
	Traversal *up_traversal;
	Turn *up_turn;
	Soup *up_soup;
	
}UnionProtocol;


#endif  /*end of the union_protocol.h*/

