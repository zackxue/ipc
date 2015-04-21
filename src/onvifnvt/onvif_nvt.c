
#include "generic.h"
#include "stdsoap2.h"
#include "http_common.h"
#include "app_debug.h"
#include "onvif_nvt.h"

SPOOK_SESSION_PROBE_t ONVIF_nvt_probe(const void* msg, ssize_t msg_sz)
{
	if(0 == strncasecmp(msg, "POST", 4)){
		HTTP_REQUEST_LINE_t request_line;
		char* const request_msg = alloca(msg_sz + 1);
		char* http_uri_suffix = NULL;

		memcpy(request_msg, msg, msg_sz);
		request_msg[msg_sz] = '\0';
		http_parse_request_line(request_msg, &request_line);

		// get suffix
		http_uri_suffix = AVAL_STRDUPA(request_line.uri_suffix);
		APP_TRACE("Check suffix: \"%s\"", http_uri_suffix);
		if(STR_CASE_THE_SAME(http_uri_suffix, "/onvif/device_service")
			|| STR_CASE_THE_SAME(http_uri_suffix, "/onvif/media")
			|| STR_CASE_THE_SAME(http_uri_suffix, "/onvif/imaging")
			|| STR_CASE_THE_SAME(http_uri_suffix, "/onvif/device")
			|| STR_CASE_THE_SAME(http_uri_suffix, "/onvif/events")
			|| STR_CASE_THE_SAME(http_uri_suffix, "/onvif/analytics")
			|| STR_CASE_THE_SAME(http_uri_suffix, "/onvif/ptz")
			|| STR_CASE_THE_SAME(http_uri_suffix, "onvif/services")){
			return SPOOK_PROBE_MATCH;
		}
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t ONVIF_nvt_loop(uint32_t* trigger, int sock, time_t* read_pts)
{
	int ret = 0;
	struct soap add_soap;
	soap_init(&add_soap);
	soap_set_namespaces(&add_soap, namespaces);
	add_soap.bind_flags = add_soap.bind_flags | SO_REUSEADDR;
	add_soap.recv_timeout = 3;
	add_soap.send_timeout = 3;
	add_soap.socket = sock;

	ret = soap_serve(&add_soap);
	soap_end(&add_soap);
	soap_done(&add_soap);

	return 0;
}

