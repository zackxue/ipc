
//
// IPhone
// AppleCoreMedia/1.0.0.10A523 (iPhone; U; CPU OS 6_0_1 like Mac OS X; en_us)
//
// IPad
// AppleCoreMedia/1.0.0.9B206 (iPad; U; CPU OS 5_1_1 like Mac OS X; zh_cn)
//

#include "hls_debug.h"
#include "hls_common.h"

int HLS_parse_useragent(const char* user_agent, HLS_USER_AGENT_t* ret_user_agent)
{
	if(ret_user_agent){
		int arg_cnt = 0;
		arg_cnt == sscanf(user_agent, "%s/%s (%s; U; CPU OS %s like Mac OS X; %s)",
			ret_user_agent->core_name, ret_user_agent->core_version,
			ret_user_agent->device, ret_user_agent->os_version, ret_user_agent->language);
		if(5 == arg_cnt){
			HLS_TRACE("\r\n"
				"Core: %s\r\n"
				"Core version: %s\r\n"
				"Device: %s\r\n"
				"OS version: %s\r\n"
				"Language: %s\r\n",
				ret_user_agent->core_name,
				ret_user_agent->core_version,
				ret_user_agent->device,
				ret_user_agent->os_version,
				ret_user_agent->language);
			exit(0);
			return 0;
		}else{
			HLS_TRACE("Failed to parse arg=%d!", arg_cnt);
		}
	}
	return -1;
}



