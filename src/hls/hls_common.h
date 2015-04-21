
//
// IPhone
// AppleCoreMedia/1.0.0.10A523 (iPhone; U; CPU OS 6_0_1 like Mac OS X; en_us)
//
// IPad
// AppleCoreMedia/1.0.0.9B206 (iPad; U; CPU OS 5_1_1 like Mac OS X; zh_cn)
//



typedef struct HLS_USER_AGENT
{
	char core_name[32];
	char core_version[16];
	char device[16];
	char os_version[32];
	char language[32];
}HLS_USER_AGENT_t;

extern int HLS_parse_useragent(const char* user_agent, HLS_USER_AGENT_t* ret_user_agent);


