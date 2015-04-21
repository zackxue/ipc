#include "esee.h"
#include "esee_protocol.h"
#include "ezxml.h"
#include "rudpa_debug.h"

EseeTag TagTable[] = {
	{ ESEE_TAG_ESEE, "esee", "char",NULL, TAG_SIGN_ESEE },
	{ ESEE_TAG_HEAD, "head", "char", NULL,TAG_SIGN_HEAD },
	{ ESEE_TAG_CMD, "cmd", "int", NULL,TAG_SIGN_HEADSUB },
	{ ESEE_TAG_TICK, "tick", "int", NULL,TAG_SIGN_HEADSUB },
	{ ESEE_TAG_PKTNUM, "pktnum", "char", NULL,TAG_SIGN_HEADSUB },
	{ ESEE_TAG_PKTNO, "pktno", "char", NULL,TAG_SIGN_HEADSUB },
	{ ESEE_TAG_SN, "sn", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_ID, "id", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_PWD, "pwd", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_STATUS, "status", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_CHANNEL, "channel", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_ERRCMD, "errcmd", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_ERRINFO, "errinfo", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_ECODE, "ecode", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_INTERIP, "interip", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_EXTERIP, "exterip", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_INTERPORT, "interport", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_EXTERPORT, "exterport", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_PORT, "port", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_DATAPORT, "dataport", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_PHONEPORT, "phoneport", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_URL, "url", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_VERSION, "version", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_VENDOR, "vendor", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_RANDOM, "random", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_NODES, "nodes", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_NODE, "node", "char", NULL,TAG_SIGN_BODYSUB },	
	{ ESEE_TAG_LENGTH, "length", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_DATA, "data", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_DVRIP, "dvrip", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_DVRPORT, "dvrport", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_CLIENTIP, "clientip", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_CLIENTPORT, "clientport", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_TURNSERVERS, "turnservers", "char", NULL,TAG_SIGN_BODY },
	{ ESEE_TAG_TURNSERVER, "turnserver", "char", NULL,TAG_SIGN_BODYSUB },

};

CmdPack CmdPackTable[] = {
	{ 0, "SRequestIdentify", "10000", { "sn", "channel", "vendor", "version" } },
	{ 1, "SRequestLogin", "10001", { "id", "interip", "interport", "port", "dataport", "phoneport" } },
	{ 2, "SHeartbeat", "10002", { "id" } },
	{ 3, "SRequestAllport", "10003", { "dataport", "phoneport" } },
	{ 4, "SRequestDataport", "10004", { "dataport" } },
	{ 5, "SRequestPhoneport", "10005", { "phoneport" } },
	{ 6, "SRequestInterip", "10006", { "interip" } },
	{ 7, "SRequestUrl", "10007" },
	{ 8, "SRequestUpdate", "10008" },
	{ 9, "SResponseIdentify", "11000", { "id" } },//"id", "pwd"
	{ 10, "SResponseLogin", "11001", { "id", "pwd", "exterip", "port" } },//"id", "pwd", "exterip", "port"
	{ 11, "SResponseHeart", "11002", { "id", "status", "exterip", "exterport",  } },
	{ 12, "SResponseAllport", "11003" },
	{ 13, "SResponseDataport", "11004" },
	{ 14, "SResponsePhoneport", "11005" },
	{ 15, "SResponseInterip", "11006" },
	{ 16, "SResponseUrl", "11007", { "url" } },
	{ 17, "SResponseUpdate", "11008", { "url", "version" } },
	{ 18, "SResponseClientInfo", "11009", { "dataport" } },//
	{ 19, "SResponseErrorInfo", "11100", { "errcmd", "ecode" } },
	{ 20, "CRequestLogin", "20001", { "id" } },//"id", "pwd"
	{ 21, "CRequestUpdate", "20002" },
	{ 22, "CResponseLogin", "21001", { "dataport" } },
	{ 23, "CResponseUpdate", "21002", { "url", "version" } },
	{ 24, "CResponseErrorInfo", "21100", { "errcmd" } },
	{ 25, "STurnAuth", "10010", { "sn" } },
	{ 26, "SResponseTurn","11010",{"clientip","clientport","turnserver"}},
	{ 27, "STurnReady","10011",{"clientip","clientport" }},
	{ 28, "STurnAck","11011", },
	{ 29, "STurnAck","21012",{"id"}},
	{ 30, "CRequestTraversal","20101",{"id","random"}},
	{ 31, "CResponseTraversal","21101",{"dvrip","dvrport","random"}},
	{ 32, "STraversalReq","11101",{"clientip","clientport","random"}},
	{ 33, "SConfirmTraversal","10101",{"clientip","clientport","random"}},
	{ 34, "CResponseConfirm","21102",{"random"}},
	{ 35, "SHoleClient","30101",{"random"}},
	{ 36, "CHoleDevice","31101",{"random"}},
};

void SetTagTxt(const char* tagName, void* tagValue)
{
	int i;
	for(i = 0; i < ESEE_MAX_TAG; ++i){
		if(EQUAL(TagTable[i].TagName, tagName)){
			FREE(TagTable[i].TagTxt);   
			COPYSTRING(TagTable[i].TagTxt, tagValue);
			break;
		}
	}
}

char * GetTagTxt(const char*tagName)
{
	int i;
	for(i = 0; i < ESEE_MAX_TAG; ++i){
		if(EQUAL(TagTable[i].TagName, tagName)){			
			return TagTable[i].TagTxt;
		}
	}
	return NULL;
}

int ReadProtocol(char* stream){
	int bufferSize = 0;
	ezxml_t pXML = ezxml_parse_str(stream, strlen(stream));
	 if(Parse(pXML , &bufferSize)){
		return bufferSize;
	}
	else{
		return -1;
	}
}

bool Parse(void* xml,  int* bufferSize){
	ezxml_t pXML = (ezxml_t)xml;
	ezxml_t pCur = pXML, pChild, pSubChild;	 
	int  nCount = 0;

	for(; NULL != pCur; pCur = pCur->sibling)
	{
		SetTagTxt( pCur->name,pCur->txt);	  
		++nCount;
		for(pChild = pCur->child; NULL != pChild; pChild = pChild->sibling)
		{
			SetTagTxt( pChild->name,pChild->txt);			
			++nCount;
			for(pSubChild = pChild->child; NULL != pSubChild; pSubChild = pSubChild->sibling)
			{
				SetTagTxt( pSubChild->name,pSubChild->txt);				
				++nCount;			
			}
		}
	}
	*bufferSize = nCount;

	return true;
}

bool FindTag(char* target){
	bool bExit = false;
	int j;
	for(j = 0; j < ESEE_MAX_TAG; ++j){
		if(!bExit && EQUAL(target, TagTable[j].TagName)){
			bExit = true;
			break;
		}
	}
	return bExit;
}
char** GetPackByCmd(char* cmd){
	int i;
	for(i = 0; i < ESEE_MAX_CMD; ++i){
		if(EQUAL(cmd, CmdPackTable[i].PackType))
			return CmdPackTable[i].PackTag;
	}
	return NULL;
}
char** GetPack(char* packName){
	int i;
	for(i = 0; i < ESEE_MAX_CMD; ++i){
		if(EQUAL(packName, CmdPackTable[i].PackName))
			return CmdPackTable[i].PackTag;
	}
	return NULL;
}
char* GetCmdByPackName(char* packName){
	int i;
	for(i = 0; i < ESEE_MAX_CMD; ++i){
		if(EQUAL(packName, CmdPackTable[i].PackName))
			return (char*)CmdPackTable[i].PackType;
	}
	return NULL;
}


char* WriteProtocol(void* packName, void* tick){
	char** ppTagPack = NULL;
	char* cmd = NULL;
	int i;
	ezxml_t xml, head, temp;

	if(NULL != packName){
		ppTagPack = GetPack((char*)packName);
		cmd = GetCmdByPackName(packName);
	}

	if(NULL  == ppTagPack || NULL == cmd)
	{
		_RUDPA_ERROR("write protocol error\n")
		return NULL;
	}
	
	xml = ezxml_new("esee");
	ezxml_set_attr(xml, "ver", "1.0");
	head = ezxml_add_child(xml, "head", 0);
	temp = ezxml_add_child(head, "cmd", 0);
	ezxml_set_txt(temp, (const char*)cmd);
	temp = ezxml_add_child(head, "tick", 1);
	ezxml_set_txt(temp, (const char*)tick);

	for(i=0; NULL!=*(ppTagPack+i); ++i)
	{
		temp = ezxml_add_child(xml, *(ppTagPack+i), i+1);			 
		ezxml_set_txt(temp, GetTagTxt(*(ppTagPack+i)));
	}
	char* data = ezxml_toxml(xml);	

	char* tmp = strdup(data);
	FREE(data);
	return tmp;
}

