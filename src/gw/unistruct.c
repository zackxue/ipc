/*
 * unistruct_public.c
 *
 *  Created on: 2011-7-12
 *      Author: root
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mempool.h"

#include "unistruct.h"
#include "unistruct_static.cxx"


UniStructDoc* UniStruct_init_doc(char* _name, char* _content)
{
	UniStructDoc* ret = MALLOC(sizeof(UniStructDoc*));
	ret->root = MALLOC(sizeof(UniStructNode));
	_unistruct_malloc_node(ret->root, NULL, _name, _content);
	return ret;
}

void UniStruct_destory(UniStructDoc* _doc)
{
	_unistruct_destory(_doc);
}

UniStructRoot* UniStruct_get_root(UniStructDoc* _doc)
{
	return _doc->root;
}

void UniStruct_loop(UniStructNode* _node,
		UniStructLoopNodeCall _node_call, UniStructLoopAttrCall _attr_call, void* _args)
{
	_unistruct_loop(_node, _node_call, _attr_call, _args);
}

int UniStruct_get_child_count(UniStructNode* _node)
{
	return LDList_get_count(_node->child);
}

UniStructNode* UniStruct_append_child(UniStructNode* _node, char* _name, char* _content)
{
	return _unistruct_append_child(_node, _name, _content);
}

UniStructNodesList* UniStruct_find_children(UniStructNode* _node, char* _name)
{
	return _unistruct_find_children(_node, _name);
}

UniStructNode* UniStruct_modify_child(UniStructNode* _node, char* _content)
{
	return _unistruct_modify_child(_node, _content);
}

UniStructNode* UniStruct_first_child(UniStructNode* _node)
{
	return _uniStruct_first_child(_node);
}

UniStructNode* UniStruct_get_child_by_index(UniStructNode* _node, int _index)
{
	return _unistruct_get_child_by_index(_node, _index);
}

UniStructNode* UniStruct_last_child(UniStructNode* _node)
{
	return _uniStruct_last_child(_node);
}

void UniStruct_free_nodes_list(UniStructNodesList* _node_list)
{
	FREE(_node_list);
}

int UniStruct_get_attr_count(UniStructNode* _node)
{
	return _unistruct_get_attr_count(_node);
}

UniStructAttr* UniStruct_append_attr(UniStructNode* _node, char* _name, char* _value)
{
	return _unistruct_append_attr(_node, _name, _value);
}

UniStructAttr* UniStruct_find_attr(UniStructNode* _node, char* _name)
{
	return _unistruct_find_attr(_node, _name);
}

UniStructAttr* UniStruct_get_attr_by_index(UniStructNode* _node, int _index)
{
	return _unistruct_get_attr_by_index(_node, _index);
}

UniStructAttr* UniStruct_modify_attr(UniStructNode* _node, char* _name, char* _value)
{
	UniStructAttr* tmp_attr = _unistruct_find_attr(_node, _name);
	if(tmp_attr != NULL)
	{
		FREE(tmp_attr->name);
		FREE(tmp_attr->value);
		tmp_attr->name = MALLOC(strlen(_name) + 1);
		strcpy(tmp_attr->name, _name);
		tmp_attr->value = MALLOC(strlen(_value) + 1);
		strcpy(tmp_attr->value, _value);
	}
	return tmp_attr;
}

UniStructAttr* UniStruct_modify_attr2(UniStructAttr* _attr, char* _value)
{
	if(_attr != NULL)
	{
		FREE(_attr->value);
		_attr->value = MALLOC(strlen(_value) + 1);
		strcpy(_attr->value, _value);
	}
	return _attr;
}

void UniStruct_delete_attr(UniStructNode* _node, char* _name)
{
	UniStructAttr* tmp_attr = _unistruct_find_attr(_node, _name);
	if(tmp_attr != NULL)
	{
		LDListItemHead* parent_item = (LDListItemHead*)((unsigned char*)tmp_attr - sizeof(LDListItemHead));
		LDList_del(_node->attr, parent_item);
	}
}

void UniStruct_to_string_setup(int _with_tab, int _with_enter)
{
	_unistruct_with_tab = _with_tab;
	_unistruct_with_enter = _with_enter;
}

int UniStruct_to_xml_string(UniStructNode* _node, char* _xml_buf)
{
	_unistruct_print_node_lv = -1;

	if(_xml_buf == NULL)
	{
		_unistruct_p_len = 0;
		_unistruct_p = NULL;
		_unistruct_dummy_p = fopen("/dev/null", "w");
		_unistruct_print_node(_node);
		fclose(_unistruct_dummy_p);
	}
	else
	{
		_unistruct_p_len = 0;
		_unistruct_p = _xml_buf;
		_unistruct_dummy_p = NULL;
		_unistruct_print_node(_node);
	}

	_unistruct_p = NULL;
	_unistruct_dummy_p = NULL;

	return _unistruct_p_len;
}

//int UniStruct_to_json_string(UniStructNode* _node, char* _json_buf, char* _jsoncallback)
//{
//	_unistruct_print_node_lv = -1;
//
//	if(_json_buf == NULL)
//	{
//		_unistruct_p_len = 0;
//		_unistruct_p = NULL;
//		_unistruct_dummy_p = fopen("/dev/null", "w");
//		_unistruct_print_begin_json(_jsoncallback);
//		_unistruct_print_node_json(_node);
//		_unistruct_print_end_json();
//		fclose(_unistruct_dummy_p);
//	}
//	else
//	{
//		_unistruct_p_len = 0;
//		_unistruct_p = _json_buf;
//		_unistruct_dummy_p = NULL;
//		_unistruct_print_begin_json(_jsoncallback);
//		_unistruct_print_node_json(_node);
//		_unistruct_print_end_json();
//	}
//
//	_unistruct_p = NULL;
//	_unistruct_dummy_p = NULL;
//
//	return _unistruct_p_len;
//}

UniStructDoc* UniStruct_from_xml_string(char* _xml)
{
	_unistruct_proc_arg proc_arg;
	proc_arg.doc = NULL;
	proc_arg.node_ptr_stack = LDList_init(sizeof(UniStructNode**));
	proc_arg.xml = _xml;
	proc_arg.ret = 0;

	int node_state = 1;
	char* p = del_space(proc_arg.xml);
	//p 绝对不会等于NULL
	//node_state 是check_state_from_n的返回值
	//proc_arg.ret 是proc_n的返回值
	while(*p != '\0' && node_state != -1 && proc_arg.ret != -1)
	{
//		UNISTRUCT_TRACE("%30.30s", p);
//		if(proc_arg.doc != NULL)
//		{
////			UniStruct_to_xml_string(UniStruct_get_root(proc_arg.doc));
//		}
//		else
//		{
//			UNISTRUCT_TRACE("no doc");
//		}
//		UNISTRUCT_TRACE("--------");

		switch(node_state)
		{
		case 1:
			p = proc_1(&proc_arg);
			break;
		case 2:
			p = proc_2(&proc_arg);
			break;
		case 3:
			p = proc_3(&proc_arg);
			break;
		case 4:
			p = proc_4(&proc_arg);
			break;
		case 5:
			p = proc_5(&proc_arg);
			break;
		case 6:
			p = proc_6(&proc_arg);
			break;
		default:
			break;
		}

		if(proc_arg.ret != 0)
		{
			break;
		}

		proc_arg.xml = del_space(proc_arg.xml);
		p = proc_arg.xml;

		switch(node_state)
		{
		case 1:
			node_state = check_state_from_1(p);
			break;
		case 2:
			node_state = check_state_from_2(p);
			break;
		case 3:
			node_state = check_state_from_3(p);
			break;
		case 4:
			node_state = check_state_from_4(p);
			break;
		case 5:
			node_state = check_state_from_5(p);
			break;
		case 6:
			node_state = check_state_from_6(p);
			break;
		default:
			break;
		}
	}

	int founderr = 0;
	if(proc_arg.ret == -1) //proc_n返回错误
	{
		founderr = 1;
	}
	if(node_state == -1 && strlen(proc_arg.xml) > 0) //check_state_from_n返回错误
	{
		founderr = 2;
	}
	if(LDList_get_count(proc_arg.node_ptr_stack) > 0) //入栈出栈数据不一致
	{
		founderr = 3;
	}
	if(founderr != 0)
	{
		UNISTRUCT_TRACE("founderr=%d", founderr);
		if(proc_arg.doc != NULL)
		{
			_unistruct_destory(proc_arg.doc);
			proc_arg.doc = NULL;
		}
	}

	LDList_destory(proc_arg.node_ptr_stack);
	return proc_arg.doc;
}


char* UniStruct_xml_to_xml(char* _xml, char* _new_xml) //测试用。不完善
{
	char* ret = _new_xml;
	int can_del_blank = 0;
	while(*_xml != '\0')
	{
		if(*_xml == '>')
		{
			can_del_blank = 1;
		}
		else if(*_xml == '<')
		{
			can_del_blank = 0;
		}

		if(can_del_blank == 1)
		{
			_xml = del_space(_xml);
		}

		*_new_xml++ = *_xml++;
	}
	return ret;
}

int UniStruct_test()
{

	return 0;
}
