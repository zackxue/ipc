/*
 * unistruct_static.c
 *
 *  Created on: 2011-7-12
 *      Author: root
 */

static void _unistruct_loop(UniStructNode* _node,
		UniStructLoopNodeCall _node_call, UniStructLoopAttrCall _attr_call, void* _args)
{
	int ret_call = 0;
	if(_node != NULL)
	{
		if(LDList_get_count(_node->attr) > 0)
		{
			int _loop_attr(LDListItemHead* _item, void* _attr_args)
			{
				UniStructAttr* attr_item = LDList_get_data(_item);
				if(_attr_call != NULL)
				{
					ret_call = _attr_call(attr_item, _attr_args);
				}
				return ret_call;
			}
			LDList_loop(LDList_get_head(_node->attr), _loop_attr, _args);
		}

		if(LDList_get_count(_node->child) > 0)
		{
			int _loop(LDListItemHead* _item, void* _node_args)
			{
				UniStructNode* node = LDList_get_data(_item);
				_unistruct_loop(node, _node_call, _attr_call, _node_args);
				return ret_call;
			}
			LDList_loop(LDList_get_head(_node->child), _loop, _args);
		}

		if(_node_call != NULL)
		{
			ret_call = _node_call(_node, _args);
		}
	}
}

static void _unistruct_malloc_node(UniStructNode* _node, UniStructNode* _parent,
		char* _name, char* _content)
{
	UNISTRUCT_ASSERT(_node != NULL, "_node is NULL");
	_node->name = MALLOC(strlen(_name) + 1);
	strcpy(_node->name, _name);
	_node->content = MALLOC(strlen(_content) + 1);
	strcpy(_node->content, _content);
	_node->parent = _parent;
	_node->child = LDList_init(sizeof(UniStructNode));;
	_node->attr = LDList_init(sizeof(UniStructAttr));
	if(_parent != NULL)
	{
		_node->path = MALLOC(strlen(_parent->path) + 1 + strlen(_name) + 1);
		sprintf(_node->path, "%s/%s", _parent->path, _name);
	}
	else
	{
		_node->path = MALLOC(1 + strlen(_name) + 1);
		sprintf(_node->path, "/%s", _name);
	}
}

//static void _unistruct_free_attr(UniStructNode* _node)
//{
//	int _loop_attr(DListItemHead* _item, void* _args)
//	{
//		UniStructAttr* attr_item = _item->data;
//		FREE(attr_item->name);
//		FREE(attr_item->value);
//		return 0;
//	}
//	DList_loop(DList_get_head(_node->attr), _loop_attr, NULL);
//}

//static void _unistruct_free_node(UniStructNode* _node)
//{
//	if(_node != NULL)
//	{
//		if(DList_get_count(_node->attr) > 0)
//		{
//			_unistruct_free_attr(_node);
//		}
//		DList_destory(_node->attr);
//
//		if(DList_get_count(_node->child) > 0)
//		{
//			int _loop(DListItemHead* _item, void* _args)
//			{
//				UniStructNode* node = _item->data;
//				_unistruct_free_node(node);
//				return 0;
//			}
//			DList_loop(DList_get_head(_node->child), _loop, NULL);
//		}
//		DList_destory(_node->child);
//
//		FREE(_node->name);
//		FREE(_node->content);
//	}
//}

static void _unistruct_destory(UniStructDoc* _doc)
{
	int _node_call(UniStructNode* _node, void* _node_args)
	{
		FREE(_node->path);
		LDList_destory(_node->attr);
		LDList_destory(_node->child);
		FREE(_node->content);
		FREE(_node->name);
		return 0;
	}
	int _attr_call(UniStructAttr* _attr, void* _attr_args)
	{
		FREE(_attr->path);
		FREE(_attr->value);
		FREE(_attr->name);
		return 0;
	}
	_unistruct_loop(_doc->root, _node_call, _attr_call, NULL);
	FREE(_doc->root);
	FREE(_doc);
}

static UniStructNode* _unistruct_append_child(UniStructNode* _node, char* _name, char* _content)
{
	UniStructNode tmp_node;
	_unistruct_malloc_node(&tmp_node, _node, _name, _content);

	LDListItemHead* tmp_item = LDList_add(_node->child, &tmp_node);
	return LDList_get_data(tmp_item);
}

static UniStructAttr* _unistruct_find_attr(UniStructNode* _node, char* _name)
{
	UniStructAttr* tmp_attr = NULL;

	struct _find_attr_arg
	{
		//input
		char* name;
		//output
		LDListItemHead* item;
	}ret;

	ret.name = _name;
	ret.item = NULL;

	int _find_attr(LDListItemHead* _item, void* _args)
	{
		struct _find_attr_arg* args = (struct _find_attr_arg*)_args;
		UniStructAttr* attr_item = _item->data;
		if(strcmp(attr_item->name, args->name) == 0)
		{
			args->item = _item;
			return -1;
		}
		return 0;
	}
	LDList_loop(LDList_get_head(_node->attr), _find_attr, &ret);

	if(ret.item != NULL)
	{
		tmp_attr = ret.item->data;
	}

	return tmp_attr;
}

static UniStructAttr* _unistruct_get_attr_by_index(UniStructNode* _node, int _index)
{
	UniStructAttr* tmp_attr = NULL;

	struct _find_attr_arg
	{
		//input
		int curr_index;
		int target_index;
		//output
		LDListItemHead* item;
	}ret;

	ret.curr_index = 0;
	ret.target_index = _index;
	ret.item = NULL;

	int _find_attr(LDListItemHead* _item, void* _args)
	{
		struct _find_attr_arg* args = (struct _find_attr_arg*)_args;

		if(args->curr_index == args->target_index)
		{
			args->item = _item;
			return -1;
		}

		args->curr_index++;
		return 0;
	}
	LDList_loop(LDList_get_head(_node->attr), _find_attr, &ret);

	if(ret.item != NULL)
	{
		tmp_attr = ret.item->data;
	}

	return tmp_attr;
}

static int _unistruct_get_attr_count(UniStructNode* _node)
{
	return LDList_get_count(_node->attr);
}

static UniStructAttr* _unistruct_append_attr(UniStructNode* _node, char* _name, char* _value)
{
	UniStructAttr* tmp_attr = NULL;

	tmp_attr = _unistruct_find_attr(_node, _name);

	if(tmp_attr != NULL) //modify data
	{
		FREE(tmp_attr->name);
		FREE(tmp_attr->value);
		tmp_attr->name = MALLOC(strlen(_name) + 1);
		strcpy(tmp_attr->name, _name);
		tmp_attr->value = MALLOC(strlen(_value) + 1);
		strcpy(tmp_attr->value, _value);
	}
	else //add data
	{
		UniStructAttr attr;
		attr.name = MALLOC(strlen(_name) + 1);
		strcpy(attr.name, _name);
		attr.value = MALLOC(strlen(_value) + 1);
		strcpy(attr.value, _value);
		attr.path = MALLOC(strlen(_node->path) + 2 + strlen(_name) + 1);
		sprintf(attr.path, "%s/@%s", _node->path, _name);

		LDListItemHead* add_ret = LDList_add(_node->attr, &attr);
		tmp_attr = add_ret->data;
	}
	return tmp_attr;
}

static UniStructNodesList* _unistruct_find_children(UniStructNode* _node, char* _name)
{
	int ret_len = sizeof(UniStructNodesList) + sizeof(UniStructNodesList*) * LDList_get_count(_node->child);
	UniStructNodesList* ret = MALLOC(ret_len);
	memset(ret, 0, ret_len);
	ret->nodes = (UniStructNode**)(ret + 1);
	struct _find_node_arg
	{
		//input
		char* name;
		//output
		UniStructNodesList* node_lists;
	}find_node_arg;

	find_node_arg.name = _name;
	find_node_arg.node_lists = ret;

	int _find_attr(LDListItemHead* _item, void* _args)
	{
		struct _find_node_arg* args = (struct _find_node_arg*)_args;
		UniStructNode* node_item = _item->data;
		if(strcmp(node_item->name, args->name) == 0)
		{
			args->node_lists->nodes[args->node_lists->count] = node_item;
			args->node_lists->count++;
		}
		return 0;
	}
	LDList_loop(LDList_get_head(_node->child), _find_attr, &find_node_arg);


	return ret;
}

static UniStructNode* _unistruct_modify_child(UniStructNode* _node, char* _content)
{
	FREE(_node->content);
	_node->content = MALLOC(strlen(_content) + 1);
	strcpy(_node->content, _content);
	return _node;
}

static UniStructNode* _uniStruct_first_child(UniStructNode* _node)
{
	UniStructNode* node_item = NULL;
	LDListItemHead* item = LDList_get_head(_node->child);
	if(item != NULL)
	{
		node_item = item->data;
	}
	return node_item;
}

static UniStructNode* _unistruct_get_child_by_index(UniStructNode* _node, int _index)
{
	int i;
	UniStructNode* node_item = NULL;
	LDListItemHead* item = LDList_get_head(_node->child);
	if(item != NULL)
	{
		for( i = 0; i < _index; i++)
		{
			item = LDList_get_next(item);
			if(item == NULL)
			{
				break;
			}
		}
		if(item != NULL)
		{
			node_item = item->data;
		}
	}
	return node_item;

}

static UniStructNode* _uniStruct_last_child(UniStructNode* _node)
{
	UniStructNode* node_item = NULL;
	LDListItemHead* item = LDList_get_tail(_node->child);
	if(item != NULL)
	{
		node_item = item->data;
	}
	return node_item;
}


//#define _unistruct_PRINT(fmt, args...) printf(fmt, ##args)
#define _unistruct_PRINT(fmt, args...) \
{\
	if(_unistruct_p != NULL) \
	{ \
		_unistruct_p_len += sprintf(_unistruct_p, fmt, ##args); \
		_unistruct_p = goto_end(_unistruct_p); \
	} \
	else \
	{ \
		_unistruct_p_len += fprintf(_unistruct_dummy_p, fmt, ##args); \
	} \
}
char* _unistruct_p = NULL;          //输出时候有缓存
FILE* _unistruct_dummy_p = NULL;    //输出时候无缓存，指向/dev/null
int _unistruct_p_len = 0;           //输出总长度
int _unistruct_print_node_lv = 0;   //当前节点深度
int _unistruct_with_tab = 0;        //是否输出tab
int _unistruct_with_enter = 0;      //是否输出换行
static char* goto_end(char* _str)
{
	char *p;
	for (p = _str; *p != '\0'; p++)
		;
	return (p);
}

static void _unistruct_print_node_attr(UniStructNode* _node)
{
	int _loop_attr(LDListItemHead* _item, void* _args)
	{
		UniStructAttr* attr_item = _item->data;
		_unistruct_PRINT(" %s=\"%s\"", attr_item->name, attr_item->value);
		return 0;
	}
	LDList_loop(LDList_get_head(_node->attr), _loop_attr, NULL);
}

//static void _unistruct_print_node_attr_json(UniStructNode* _node)
//{
//	int first = 1;
//	int _loop_attr(LDListItemHead* _item, void* _args)
//	{
//		UniStructAttr* attr_item = _item->data;
//		if(first == 1)
//		{
//			first = 0;
//			_unistruct_PRINT(" \"%s\":\"%s\"", attr_item->name, attr_item->value);
//		}
//		else
//		{
//			_unistruct_PRINT(", \"%s\":\"%s\"", attr_item->name, attr_item->value);
//		}
//		return 0;
//	}
//	LDList_loop(LDList_get_head(_node->attr), _loop_attr, NULL);
//}

static void _unistruct_print_node_tail(UniStructNode* _node)
{
	if(LDList_get_count(_node->child) > 0)
	{
		if(_unistruct_with_tab == 1)
		{
			int i;
			for(i = 0; i < _unistruct_print_node_lv; i++)
			{
				_unistruct_PRINT("\t");
			}
		}
		_unistruct_PRINT("</%s>", _node->name);
		if(_unistruct_with_enter == 1)
		{
			_unistruct_PRINT("\r\n");
		}
	}
}

//static void _unistruct_print_node_tail_json(UniStructNode* _node)
//{
//	if(LDList_get_count(_node->child) > 0 || LDList_get_count(_node->attr) > 0)
//	{
//		if(_unistruct_with_tab == 1)
//		{
//			int i;
//			for(i = 0; i < _unistruct_print_node_lv; i++)
//			{
//				_unistruct_PRINT("\t");
//			}
//		}
//		_unistruct_PRINT("}");
//		if(_node->parent != NULL && _uniStruct_last_child(_node->parent) != _node)
//		{
//			_unistruct_PRINT(",");
//		}
//		if(_unistruct_with_enter == 1)
//		{
//			_unistruct_PRINT("\r\n");
//		}
//	}
//}

static void _unistruct_print_node_head(UniStructNode* _node)
{
	if(_unistruct_with_tab == 1)
	{
		int i;
		for(i = 0; i < _unistruct_print_node_lv; i++)
		{
			_unistruct_PRINT("\t");
		}
	}
	_unistruct_PRINT("<%s", _node->name);
	if(LDList_get_count(_node->attr) > 0)
	{
		_unistruct_print_node_attr(_node);
	}

	if(strlen(_node->content) == 0 && LDList_get_count(_node->child) == 0)
	{
		_unistruct_PRINT("></%s>", _node->name);
	}
	else
	{
		_unistruct_PRINT(">%s", _node->content);
		if(LDList_get_count(_node->child) == 0)
		{
			_unistruct_PRINT("</%s>", _node->name);
		}
	}
	if(_unistruct_with_enter == 1)
	{
		_unistruct_PRINT("\r\n");
	}
}

//static void _unistruct_print_node_head_json(UniStructNode* _node)
//{
//	if(_unistruct_with_tab == 1)
//	{
//		int i;
//		for(i = 0; i < _unistruct_print_node_lv; i++)
//		{
//			_unistruct_PRINT("\t");
//		}
//	}
//	_unistruct_PRINT("%s:{", _node->name);
//	if(LDList_get_count(_node->attr) > 0)
//	{
//		_unistruct_print_node_attr_json(_node);
//	}
//
//	if(LDList_get_count(_node->child) > 0)
//	{
//		_unistruct_PRINT(",");
//	}
//	if(_unistruct_with_enter == 1)
//	{
//		_unistruct_PRINT("\r\n");
//	}
//}

static void _unistruct_print_node(UniStructNode* _node)
{
	_unistruct_print_node_lv ++;
	if(_node != NULL)
	{
		_unistruct_print_node_head(_node);
		if(LDList_get_count(_node->child) > 0)
		{
			int _loop(LDListItemHead* _item, void* _args)
			{
				UniStructNode* node = _item->data;
				_unistruct_print_node(node);
				return 0;
			}
			LDList_loop(LDList_get_head(_node->child), _loop, NULL);
		}
		_unistruct_print_node_tail(_node);
	}
	_unistruct_print_node_lv--;
}

//static void _unistruct_print_begin_json(char* _jsoncallback)
//{
//	_unistruct_PRINT("%s({", _jsoncallback);
//}

//static void _unistruct_print_end_json()
//{
//	_unistruct_PRINT("})");
//}

//static void _unistruct_print_node_json(UniStructNode* _node)
//{
//	_unistruct_print_node_lv ++;
//	if(_node != NULL)
//	{
//		_unistruct_print_node_head_json(_node);
//		if(LDList_get_count(_node->child) > 0)
//		{
//			int _loop(LDListItemHead* _item, void* _args)
//			{
//				UniStructNode* node = _item->data;
//				_unistruct_print_node_json(node);
//				return 0;
//			}
//			LDList_loop(LDList_get_head(_node->child), _loop, NULL);
//		}
//		_unistruct_print_node_tail_json(_node);
//	}
//	_unistruct_print_node_lv--;
//}


static int is_space(int c) /*   是空否   */
{
	switch (c)
	{
		case 0x20:
		case 0xD:
		case 0xA:
		case 0x9:
		return 1;
	}
	return 0;
}

static char* del_space(char* _str) /* 删除前导空 */
{
	char *p;
	for (p = _str; is_space(*p); p++)
		; /* 去掉空字符 */
	return (p);
}

static int is_name_char(int c) /* 有效的名称前导符   */
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
}

static int is_content_char(int c)
{
	return (c >= 0x20 && c <= 0x7E && c != '<');
}

//#define MAX_NODE_NAME_LEN (256)
//#define MAX_ATTR_NAME_LEN (256)
//#define MAX_VALUE_NAME_LEN (256)
//#define MAX_CONTENT_LEN (256)

//1 找到<
//2 找到属性
//3 找到>
//4 找到content
//5 找到/>
//6 找到</tagname>


//找到<
static char* proc_1(_unistruct_proc_arg* _arg)
{
	UNISTRUCT_ASSERT(*_arg->xml == '<', "_arg->xml=%s", _arg->xml);
	_arg->ret = 0;
	_arg->xml++; //<

	if(*_arg->xml == '!')
	{
		char* misc_tail = strstr(_arg->xml, "-->");
		if(misc_tail == NULL)
		{
			_arg->ret = -1;
			return _arg->xml;
		}
		misc_tail += 3;
		_arg->xml += misc_tail - _arg->xml;
	}
	else if(*_arg->xml == '?')
	{
		char* misc_tail = strstr(_arg->xml, "?>");
		if(misc_tail == NULL)
		{
			_arg->ret = -1;
			return _arg->xml;
		}
		misc_tail += 2;
		_arg->xml += misc_tail - _arg->xml;
	}
	else
	{
		int node_len = strcspn(_arg->xml, " >/");
		//todo:找不到，直接到了字符串最后，如何判断错误
		char* node_name = MALLOC(node_len + 1);
//		memset(node_name, 0, node_len + 1);
		strncpy(node_name, _arg->xml, node_len);
		node_name[node_len] = 0;
		_arg->xml += node_len;

		LDListItemHead* item = LDList_get_tail(_arg->node_ptr_stack);
		if(item == NULL) //root node
		{
			_arg->doc = MALLOC(sizeof(UniStructDoc*));
			_arg->doc->root = MALLOC(sizeof(UniStructNode));
			_unistruct_malloc_node(_arg->doc->root, NULL, node_name, "");

			LDList_add(_arg->node_ptr_stack, &(_arg->doc->root));
		}
		else
		{
			UniStructNode* node = *((UniStructNode**)item->data);
			UniStructNode* new_node = _unistruct_append_child(node, node_name, "");
			LDList_add(_arg->node_ptr_stack, &new_node);
		}
		FREE(node_name);
	}

	return _arg->xml;
}

static int check_state_from_1(char* _c)
{
	if(is_name_char(*_c)) //<node
	{
		return 2;
	}
	else if(*_c == '>') // node>
	{
		return 3;
	}
	else if(*_c == '/' && *(_c+1) == '>') // node/>
	{
		return 5;
	}
	else // <? ?>< or <! > ignore filehead and commet
	{
		if(*_c == '<' && *(_c+1) == '/') // <? ?></old_node> or <! ></old_node>
		{
			return 6;
		}
		else if(*_c == '<') // <? ?>< or <! ><
		{
			return 1;
		}
	}
	return -1;
}

//属性开始
static char* proc_2(_unistruct_proc_arg* _arg)
{
	_arg->ret = 0;

	int attr_len = strcspn(_arg->xml, "=");
	char* attr = MALLOC(attr_len + 1);
//	memset(attr, 0, attr_len + 1);
	strncpy(attr, _arg->xml, attr_len);
	attr[attr_len] = 0;
	_arg->xml += attr_len + 1 + 1;

	int value_len = strcspn(_arg->xml, "\"");
	char* value = MALLOC(value_len + 1);
//	memset(value, 0, value_len + 1);
	strncpy(value, _arg->xml, value_len);
	value[value_len] = 0;
	_arg->xml += value_len + 1;

	LDListItemHead* item = LDList_get_tail(_arg->node_ptr_stack);
	UNISTRUCT_ASSERT(item != NULL, "item is NULL");
	UniStructNode* node = *((UniStructNode**)item->data);

	_unistruct_append_attr(node, attr, value);

	FREE(attr);
	FREE(value);
	return _arg->xml;
}

static int check_state_from_2(char* _c)
{
	if(is_name_char(*_c)) //attr="value" attr2="value2"
	{
		return 2;
	}
	else if(*_c == '>') //attr="value">
	{
		return 3;
	}
	else if(*_c == '/' && *(_c+1) == '>') //attr="value"/>
	{
		return 5;
	}
	return -1;
}

//找到>
static char* proc_3(_unistruct_proc_arg* _arg)
{
	_arg->ret = 0;

	_arg->xml++;

	return _arg->xml;
}

static int check_state_from_3(char* _c)
{
	if(is_content_char(*_c)) //>content
	{
		return 4;
	}
	else if(*_c == '<' && *(_c+1) == '/') //></node>
	{
		return 6;
	}
	else if(*_c == '<' && *(_c+1) == '!') //><!--
	{
		return 1;
	}
	else if(*_c == '<') //><new_node
	{
		return 1;
	}
	return -1;
}

//找到content
static char* proc_4(_unistruct_proc_arg* _arg)
{
	_arg->ret = 0;

	int content_len = strcspn(_arg->xml, "<");
	char* content = MALLOC(content_len + 1);
//	memset(content, 0, content_len + 1);
	strncpy(content, _arg->xml, content_len);
	content[content_len] = 0;
	_arg->xml += content_len;

	LDListItemHead* item = LDList_get_tail(_arg->node_ptr_stack);
	UNISTRUCT_ASSERT(item != NULL, "item is NULL");
	UniStructNode* node = *((UniStructNode**)item->data);

	_unistruct_modify_child(node, content);

	FREE(content);
	return _arg->xml;
}

static int check_state_from_4(char* _c)
{
	if(*_c == '<' && *(_c+1) == '/')//content</node>
	{
		return 6;
	}
	else if(*_c == '<') //content<new_node
	{
		return 1;
	}
	return -1;
}

//找到/>
static char* proc_5(_unistruct_proc_arg* _arg)
{
	_arg->ret = 0;

	_arg->xml += 2; // />

	LDListItemHead* item = LDList_get_tail(_arg->node_ptr_stack);
	UNISTRUCT_ASSERT(item != NULL, "item is NULL");
	LDList_del(_arg->node_ptr_stack, item);

	return _arg->xml;
}

static int check_state_from_5(char* _c)
{
	if(*_c == '<' && *(_c+1) == '/') // /></old_node>
	{
		return 6;
	}
	else if(*_c == '<') // /><new_node
	{
		return 1;
	}
	return -1;
}

//找到</tag>
static char* proc_6(_unistruct_proc_arg* _arg)
{
	_arg->ret = 0;

	_arg->xml += 2; //</

	int content_len = strcspn(_arg->xml, ">");
	char* content = MALLOC(content_len + 1);
//	memset(content, 0, content_len + 1);
	strncpy(content, _arg->xml, content_len);
	content[content_len] = 0;
	_arg->xml += content_len + 1;

	LDListItemHead* item = LDList_get_tail(_arg->node_ptr_stack);
	UNISTRUCT_ASSERT(item != NULL, "item is NULL");
	LDList_del(_arg->node_ptr_stack, item);

	FREE(content);
	return _arg->xml;
}

static int check_state_from_6(char* _c)
{
	if(*_c == '<' && *(_c+1) == '/') //</node></old_node>
	{
		return 6;
	}
	else if(*_c == '<') // </node><new_node
	{
		return 1;
	}
	return -1;
}


