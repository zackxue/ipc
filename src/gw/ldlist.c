/*
 * dlist.c
 *
 *  Created on: 2011-7-9
 *      Author: root
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//#include "mempool.h"
#include "ldlist.h"

#define MALLOC(size) malloc(size)
#define FREE(addr) free(addr)


#define LDLIST_ASSERT(express, fmt, args...) assert(((express) ? 1 : (fprintf(stderr, fmt"\t", ##args),0)))
#define LDLIST_TRACE(fmt, args...) printf(fmt"\n", ##args)

static int _ldlist_loop(LDListItemHead* _item_begin, LDListLoopCall _call, void* _args)
{
	LDListItemHead* curr = _item_begin;
	int call_ret = 0;
	while(curr != NULL)
	{
		if(_call != NULL)
		{
			call_ret = _call(curr, _args);
			if(call_ret != 0)
			{
				break;
			}
		}
		curr = curr->next;
	}
	return call_ret;
}

LDList* LDList_init(int _item_size)
{
	LDList* ret = (LDList*)MALLOC(sizeof(LDList));
	ret->item_size = _item_size;
	ret->item_count = 0;
	ret->head = NULL;
	ret->tail = NULL;
	return ret;
}

LDListItemHead* LDList_add(LDList* _ldlist, void* _data)
{
	LDListItemHead* tmp;

	tmp = (LDListItemHead*)MALLOC(sizeof(LDListItemHead) + _ldlist->item_size);
	tmp->size = _ldlist->item_size;
	tmp->next = NULL;
	tmp->prev = _ldlist->tail;

	if(_ldlist->item_count == 0)
	{
		_ldlist->head = tmp;
	}
	else
	{
		_ldlist->tail->next = tmp;
	}
	_ldlist->tail = tmp;
	_ldlist->item_count ++;


	tmp->data = tmp + 1;
	memcpy(tmp->data, _data, _ldlist->item_size);
	return tmp;
}

void LDList_del(LDList* _ldlist, LDListItemHead* _item)
{

	LDListItemHead* prev_item = _item->prev;
	LDListItemHead* next_item = _item->next;

	if(prev_item != NULL)
	{
		prev_item->next = next_item;
	}
	else
	{
		_ldlist->head = next_item;
	}

	if(next_item != NULL)
	{
		next_item->prev = prev_item;
	}
	else
	{
		_ldlist->tail = prev_item;
	}

	_ldlist->item_count--;

	FREE(_item);
}

LDListItemHead* LDList_find_data(LDList* _ldlist, void* _args)
{
	struct _ldlist_find
	{
		LDListItemHead* output;
		void* input;
	}ret;

	ret.output = NULL;
	ret.input = _args;

	int _ldlist_find_call(LDListItemHead* _item, void* _args)
	{
		struct _ldlist_find* args = (struct _ldlist_find*)_args;
		if(memcmp(_item->data, args->input, _item->size) == 0)
		{
			args->output = _item;
			return -1;
		}
		return 0;
	}
	_ldlist_loop(_ldlist->head, _ldlist_find_call, &ret);

	return ret.output;
}

LDListItemHead* LDList_get_head(LDList* _ldlist)
{
	LDListItemHead* ret = NULL;
	if(_ldlist->item_count > 0)
	{
		ret = _ldlist->head;
	}
	return ret;
}

LDListItemHead* LDList_get_tail(LDList* _ldlist)
{
	LDListItemHead* ret = NULL;
	if(_ldlist->item_count > 0)
	{
		ret = _ldlist->tail;
	}
	return ret;
}

LDListItemHead* LDList_get_prev(LDListItemHead* _item)
{
	LDLIST_ASSERT(_item != NULL, "_item is NULL");
	return _item->prev;
}

LDListItemHead* LDList_get_next(LDListItemHead* _item)
{
	LDLIST_ASSERT(_item != NULL, "_item is NULL");
	return _item->next;
}

void* LDList_get_data(LDListItemHead* _item)
{
	return _item->data;
}

void LDList_copy_data(LDListItemHead* _item, void* _dest)
{
	memcpy(_dest, _item->data, _item->size);
}

int LDList_get_count(LDList* _ldlist)
{
	LDLIST_ASSERT(_ldlist != NULL, "_list is NULL");
	return _ldlist->item_count;
}

int LDList_loop(LDListItemHead* _item_begin, LDListLoopCall _call, void* _args)
{
//	DLIST_TRACE("loop begin");
	return _ldlist_loop(_item_begin, _call, _args);
}

void LDList_del_all(LDList* _ldlist)
{
	LDListItemHead* curr;
	while(_ldlist->head != NULL)
	{
		curr = _ldlist->head;

		_ldlist->head = curr->next;
		_ldlist->item_count--;
		if(_ldlist->item_count == 0)
		{
			_ldlist->tail = NULL;
		}
		FREE(curr);
	}
}

void LDList_destory(LDList* _ldlist)
{
	LDListItemHead* curr;
	while(_ldlist->head != NULL)
	{
		curr = _ldlist->head;

		_ldlist->head = curr->next;
		_ldlist->item_count--;
		if(_ldlist->item_count == 0)
		{
			_ldlist->tail = NULL;
		}
		FREE(curr);
	}
	FREE(_ldlist);
}

void LDList_print(LDList* _ldlist)
{
	LDLIST_TRACE("%s, list count=%d", __FUNCTION__, _ldlist->item_count);
	int _ldlist_print_call(LDListItemHead* _item, void* _args)
	{
		LDLIST_TRACE("item index=%u", (size_t)_item->data);
		return 0;
	}

	_ldlist_loop(_ldlist->head, _ldlist_print_call, NULL);
}

