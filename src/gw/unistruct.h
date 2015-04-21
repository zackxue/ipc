/*
 * unistruct.h
 *
 *  Created on: 2011-7-12
 *      Author: root
 */

#ifndef UNISTRUCT_H_
#define UNISTRUCT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#include "ldlist.h"

typedef struct _UniStructNode UniStructNode;
typedef struct _UniStructAttr UniStructAttr;
typedef UniStructNode UniStructRoot;
typedef struct _UniStructDoc UniStructDoc;
typedef struct _UniStructNodesList UniStructNodesList;

struct _UniStructNode
{
	char* name;
	char* content;
	struct _UniStructNode* parent;
	LDList* child; //UniStructNode
	LDList* attr; //UniStructAttr
	char* path;
};

struct _UniStructAttr
{
	char* name;
	char* value;
	char* path;
};

struct _UniStructDoc
{
	UniStructRoot* root;
};

struct _UniStructNodesList
{
	int count;
	UniStructNode** nodes;
};

typedef int(*UniStructLoopNodeCall)(UniStructNode* _node, void* _args);
typedef int(*UniStructLoopAttrCall)(UniStructAttr* _attr, void* _args);

extern UniStructDoc*       UniStruct_init_doc(char* _name, char* _content);
extern void                UniStruct_destory(UniStructDoc* _doc);
extern UniStructRoot*      UniStruct_get_root(UniStructDoc* _doc);
extern void                UniStruct_loop(UniStructNode* _node,
		UniStructLoopNodeCall _node_call, UniStructLoopAttrCall _attr_call, void* _args);

extern int                 UniStruct_get_child_count(UniStructNode* _node);
extern UniStructNode*      UniStruct_append_child(UniStructNode* _node, char* _name, char* _content);
extern UniStructNodesList* UniStruct_find_children(UniStructNode* _node, char* _name);
extern UniStructNode*      UniStruct_modify_child(UniStructNode* _node, char* _content);
extern UniStructNode*      UniStruct_first_child(UniStructNode* _node);
extern UniStructNode*      UniStruct_get_child_by_index(UniStructNode* _node, int _index);
extern UniStructNode*      UniStruct_last_child(UniStructNode* _node);
extern void                UniStruct_free_nodes_list(UniStructNodesList* _node_list);

extern int                 UniStruct_get_attr_count(UniStructNode* _node);
extern UniStructAttr*      UniStruct_append_attr(UniStructNode* _node, char* _name, char* _value);
extern UniStructAttr*      UniStruct_find_attr(UniStructNode* _node, char* _name);
extern UniStructAttr*      UniStruct_get_attr_by_index(UniStructNode* _node, int _index);
extern UniStructAttr*      UniStruct_modify_attr(UniStructNode* _node, char* _name, char* _value);
extern UniStructAttr*      UniStruct_modify_attr2(UniStructAttr* _attr, char* _value);
extern void                UniStruct_delete_attr(UniStructNode* _node, char* _name);

extern void                UniStruct_to_string_setup(int _with_tab, int _with_enter);
extern int                 UniStruct_to_xml_string(UniStructNode* _node, char* _xml_buf);
//extern int                 UniStruct_to_json_string(UniStructNode* _node, char* _json_buf, char* _jsoncallback);
extern UniStructDoc*       UniStruct_from_xml_string(char* _xml);

extern char*               UniStruct_xml_to_xml(char* _xml, char* _new_xml);
extern int                 UniStruct_test();


typedef struct __unistruct_proc_arg
{
	UniStructDoc* doc;
	LDList* node_ptr_stack;
	char* xml;
	int ret;
}_unistruct_proc_arg;



#define UNISTRUCT_ASSERT(express, fmt, args...) assert(((express) ? 1 : (fprintf(stderr, fmt"\t", ##args),0)))
#define UNISTRUCT_TRACE(fmt, args...) printf(fmt"\n", ##args)

#ifdef __cplusplus
}
#endif
#endif /* UNISTRUCT_H_ */
