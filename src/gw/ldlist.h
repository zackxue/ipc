/*
 * dlist.h
 *
 *  Created on: 2011-7-9
 *      Author: root
 */

#ifndef LDLIST_H_
#define LDLIST_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _LDListItemHead
{
	struct _LDListItemHead* next;
	struct _LDListItemHead* prev;
	int size;
	void* data;
}LDListItemHead;

typedef struct _LDList
{
	int item_size;
	int item_count;
	LDListItemHead* head;
	LDListItemHead* tail;
}LDList;

typedef int(*LDListLoopCall)(LDListItemHead* _item, void* _args);


extern LDList*          LDList_init(int _item_size);
extern void             LDList_destory(LDList* _list);
extern void             LDList_print(LDList* _list);
extern int              LDList_loop(LDListItemHead* _item_begin, LDListLoopCall _call, void* _args);
extern LDListItemHead*  LDList_add(LDList* _list, void* _data);
extern void             LDList_del(LDList* _list, LDListItemHead* _item);
extern void             LDList_del_all(LDList* _list);
extern LDListItemHead*  LDList_find_data(LDList* _list, void* _args);

extern LDListItemHead*  LDList_get_head(LDList* _dlist);
extern LDListItemHead*  LDList_get_tail(LDList* _dlist);
extern LDListItemHead*  LDList_get_prev(LDListItemHead* _item);
extern LDListItemHead*  LDList_get_next(LDListItemHead* _item);
extern void*            LDList_get_data(LDListItemHead* _item);
extern void             LDList_copy_data(LDListItemHead* _item, void* _dest);
extern int              LDList_get_count(LDList* _list);


#ifdef __cplusplus
}
#endif
#endif /* LDLIST_H_ */
