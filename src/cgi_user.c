
#include "aval.h"
#include "generic.h"
#include "inifile.h"
#include "usrm.h"
#include "jsocket.h"
#include "ezxml.h"

#include "app_debug.h"
#include "cgi_user.h"

//
// XML template
//
//

static int user_list_xml(USRM_I_KNOW_U_t* i_m, ezxml_t xml_root, lpINI_PARSER ini_list)
{
	int i = 0;
	int ret = 0;
	ezxml_t list_node = NULL;
	ezxml_t user_node = NULL;
	char str_val[32] = {""};
	
	int const user_count = ini_list->read_int(ini_list, "OPTION", "user", 0);
	APP_ASSERT(user_count > 0, "Why there is no user");

	// xml root node
	list_node = ezxml_add_child_d(xml_root, "user_list", 0); // user list node
	// attribute count
	sprintf(str_val, "%d", user_count);
	ezxml_set_attr_d(list_node, "count", str_val);
	// attribute backlog
	sprintf(str_val, "%d", USR_MANGER_USER_HOURSE_BACKLOG);
	ezxml_set_attr_d(list_node, "backlog", str_val);

	for(i = 0; NULL != i_m && i < user_count && i < USR_MANGER_USER_HOURSE_BACKLOG; ++i){
		char user_section[32] = {""};
		char user_node_name[32] = {""};
		char his_name[32] = {""};
		char *can_del_user, *can_edit_user, *can_set_pass;
		bool his_is_admin;
		char buf[1024] = {""};

		sprintf(user_section, "USER%d", i);
		sprintf(user_node_name, "user%d", i);
		//APP_TRACE("Reading section [%s]", user_section);
		
		user_node = ezxml_add_child_d(list_node, user_node_name, i); // add user child to list
		strncpy(his_name, ini_list->read_text(ini_list, user_section, "name", "", buf, sizeof(buf)), ARRAY_ITEM(his_name));
		his_is_admin = ini_list->read_bool(ini_list, user_section, "admin", false);
		ezxml_set_attr_d(user_node, "name", his_name);
		ezxml_set_attr_d(user_node, "admin", ini_list->read_bool(ini_list, user_section, "admin", false) ? "yes" : "no");
		ezxml_set_attr_d(user_node, "permit_live", ini_list->read_bool(ini_list, user_section, "permit_live", false) ? "yes" : "no");
		ezxml_set_attr_d(user_node, "permit_setting", ini_list->read_bool(ini_list, user_section, "permit_setting", false) ? "yes" : "no");
		ezxml_set_attr_d(user_node, "permit_playback", ini_list->read_bool(ini_list, user_section, "permit_playback", false) ? "yes" : "no");
		
		// 1. only admin and not user himself could be deleted
		can_del_user = "no";
		if(i_m->is_admin && !STR_CASE_THE_SAME(i_m->username, his_name)){
			can_del_user = "yes";
		}
		// 2. only admin to edit not admin, and not user himself
		can_edit_user = "no";

		if(i_m->is_admin && !his_is_admin && !STR_CASE_THE_SAME(i_m->username, his_name)){
			can_edit_user = "yes";
		}
		// 3. only user himself could set his own password
		can_set_pass = STR_CASE_THE_SAME(i_m->username, his_name) ? "yes" : "no";

		// 4. add attributes
		ezxml_set_attr_d(user_node, "del_user", can_del_user);
		ezxml_set_attr_d(user_node, "edit_user", can_edit_user);
		ezxml_set_attr_d(user_node, "set_pass", can_set_pass);

	}

	return 0;
}

static int cgi_user_http_response(HTTPD_SESSION_t* http_session, const char* xml_text)
{
	int ret = 0;
	char response_buf[4096] = {""};
	
	const char* const http_version = AVAL_STRDUPA(http_session->request_line.version);
	HTTP_HEADER_t* http_header = NULL;
	
	// response
	http_header = http_response_header_new(http_version, 200, NULL);
	if(xml_text && strlen(xml_text) > 0){
		// with content
		http_header->add_tag_text(http_header, "Content-Type", http_get_file_mime("xml"));
		http_header->add_tag_int(http_header, "Content-Length", strlen(xml_text));
	}
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_response_header_free(http_header);
	http_header = NULL;

	// make a whole tcp packet
	strncat(response_buf, xml_text, ARRAY_ITEM(response_buf));
	ret = jsock_send(http_session->sock, response_buf, strlen(response_buf));
	if(ret < 0){
		// FIXME:
	}
	return ret;
}

static int cgi_user_parse_query_string(HTTPD_SESSION_t* http_session, char ret_query_string[2048], AVal* ret_username, AVal* ret_password, AVal* ret_content)
{
	const char* const query_str_enc = AVAL_STRDUPA(http_session->request_line.uri_query_string);

	// decode the query string
	http_url_decode(query_str_enc, strlen(query_str_enc), ret_query_string, 2048);

	// get current session username / password
	if(ret_username){
		if(0 != http_read_query_string(ret_query_string, "username", ret_username)){
			// FIXME:
		}
	}
	if(ret_password){
		if(0 != http_read_query_string(ret_query_string, "password", ret_password)){
			// FIXME:
		}
	}
	if(ret_content){
		if(0 != http_read_query_string(ret_query_string, "content", ret_content)){
			// FIXME:
		}
	}

	return 0;
}

int CGI_user_list(HTTPD_SESSION_t* http_session)
{
	int i = 0;
	int ret = 0;
	AVal av_username = AVC(""), av_password = AVC("");
	
	char query_string[2048] = {""};
	
	USRM_I_KNOW_U_t* i_m = NULL;
	ezxml_t user_xml = NULL;
	const char* xml_text = NULL;
	
	int user_backlog = 0;

	// get current session username / password
	cgi_user_parse_query_string(http_session, query_string, &av_username, &av_password, NULL);

	// xml root node
	user_xml = ezxml_new_d("user");
	ezxml_set_attr_d(user_xml, "ver", CGI_USER_VERSION);

	// user check in
	i_m = USRM_login(AVAL_STRDUPA(av_username), AVAL_STRDUPA(av_password));
	if(i_m){
		lpINI_PARSER user_ini = NULL;
		
		// attribute count
		ezxml_set_attr_d(user_xml, "you", i_m->username);
		// attribute 'add user' permit
		ezxml_set_attr_d(user_xml, "add_user", i_m->is_admin ? "yes" : "no");
		
		// open the ini file
		user_ini = OpenIniFile(USR_MANGER_TMP_FILE);
		APP_ASSERT(NULL != user_ini, "File not existed? it's impossible");

		// put the user list to xml
		user_list_xml(i_m, user_xml, user_ini);

		// close the ini file
		CloseIniFile(user_ini);
		user_ini = NULL;

		// add return
		ezxml_set_attr_d(user_xml, "ret", "success");
		ezxml_set_attr_d(user_xml, "mesg", "check in success");

		USRM_logout(i_m);
		i_m = NULL;

	}else{
		// add return
		ezxml_set_attr_d(user_xml, "ret", "sorry");
		ezxml_set_attr_d(user_xml, "mesg", "check in falied");
	
	}

	// make the xml text
	xml_text = ezxml_toxml(user_xml);
	ezxml_free(user_xml);
	user_xml = NULL;

	// response
	cgi_user_http_response(http_session, xml_text);

	// free the xml text
	free(xml_text);
	xml_text = NULL;

	return 0;
}

int CGI_add_user(HTTPD_SESSION_t* http_session)
{
	int ret = 0;
	AVal av_username = AVC(""), av_password = AVC(""), av_content = AVC("");

	USRM_I_KNOW_U_t* i_m = NULL;
	bool check_in = false;
	bool add_success = false;
	ezxml_t output_xml = NULL;
	const char* xml_text = NULL;

	char query_string[2048] = {""};

	// get current session username / password
	cgi_user_parse_query_string(http_session, query_string, &av_username, &av_password, &av_content);

	// user check in
	i_m = USRM_login(AVAL_STRDUPA(av_username), AVAL_STRDUPA(av_password));
	if(i_m){
		ezxml_t input_xml = NULL;
		ezxml_t add_node = NULL;

		APP_TRACE("Login success! Query string = \"%s\"", query_string);
		
		// check in success
		check_in = true;

		input_xml = ezxml_parse_str(av_content.av_val, av_content.av_len);
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t add_node = ezxml_child(input_xml, "add_user");
			if(add_node){
				const char* attr_name = ezxml_attr(add_node, "name");
				const char* attr_password = ezxml_attr(add_node, "password");
				const char* attr_admin = ezxml_attr(add_node, "admin");
				const char* attr_permit_live = ezxml_attr(add_node, "permit_live");
				const char* attr_permit_setting = ezxml_attr(add_node, "permit_setting");
				const char* attr_permit_playback = ezxml_attr(add_node, "permit_playback");

				bool const is_admin = attr_admin ? (STR_CASE_THE_SAME(attr_admin, "yes")) : false;
				uint32_t permit_flag = 0; // clear flag

				if(attr_permit_live ? (STR_CASE_THE_SAME(attr_permit_live, "permit_live")) : false){
					permit_flag |= USRM_PERMIT_LIVE;
				}
				if(attr_permit_setting ? (STR_CASE_THE_SAME(attr_permit_setting, "permit_setting")) : false){
					permit_flag |= USRM_PERMIT_SETTING;
				}
				if(attr_permit_playback ? (STR_CASE_THE_SAME(attr_permit_playback, "permit_playback")) : false){
					permit_flag |= USRM_PERMIT_PLAYBACK;
				}

				how_about = i_m->add_user(i_m, attr_name, attr_password, is_admin, permit_flag);
				if(USRM_GREAT == how_about){
					add_success = true;
					APP_TRACE("Add user \"%s\" success!", attr_name);
					USRM_store();
				}else{
					// FIXME:

					
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", AVAL_STRDUPA(av_username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	xml_text = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(http_session, xml_text);
	
	free(output_xml);
	output_xml = NULL;
	
	return 0;
}

int CGI_del_user(HTTPD_SESSION_t* http_session)
{
	int ret = 0;
	AVal av_username = AVC(""), av_password = AVC(""), av_content = AVC("");
	USRM_I_KNOW_U_t* i_m = NULL;
	bool checkin_success = false;
	bool del_success = false;
	ezxml_t output_xml = NULL;
	char* response_xml = NULL;
	
	char query_string[2048] = {""};
	
	// get current session username / password
	cgi_user_parse_query_string(http_session, query_string, &av_username, &av_password, &av_content);

	// user check in
	i_m = USRM_login(AVAL_STRDUPA(av_username), AVAL_STRDUPA(av_password));
	if(i_m){
		ezxml_t input_xml = NULL;
		checkin_success = true;

		APP_TRACE("Login success! Query string = \"%s\"", query_string);

		input_xml = ezxml_parse_str(av_content.av_val, av_content.av_len);
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t add_node = ezxml_child(input_xml, "del_user");
			if(add_node){
				const char* attr_name = ezxml_attr(add_node, "name");

				how_about = i_m->del_user(i_m, attr_name);
				if(USRM_GREAT == how_about){
					del_success = true;
					APP_TRACE("Delete user \"%s\" success!", attr_name);
					USRM_store();
				}else{
					// FIXME:

					
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", AVAL_STRDUPA(av_username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	response_xml = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(http_session, response_xml);
	
	free(output_xml);
	output_xml = NULL;
	
	return 0;
}

int CGI_edit_user(HTTPD_SESSION_t* http_session)
{
	int ret = 0;
	AVal av_username = AVC(""), av_password = AVC(""), av_content = AVC("");

	USRM_I_KNOW_U_t* i_m = NULL;
	bool check_in = false;
	bool add_success = false;
	ezxml_t output_xml = NULL;
	const char* xml_text = NULL;

	char query_string[2048] = {""};

	// get current session username / password
	cgi_user_parse_query_string(http_session, query_string, &av_username, &av_password, &av_content);

	// user check in
	i_m = USRM_login(AVAL_STRDUPA(av_username), AVAL_STRDUPA(av_password));
	if(i_m){
		ezxml_t input_xml = NULL;
		ezxml_t add_node = NULL;

		APP_TRACE("Login success! Query string = \"%s\"", query_string);
		
		// check in success
		check_in = true;

		input_xml = ezxml_parse_str(av_content.av_val, av_content.av_len);
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t edit_node = ezxml_child(input_xml, "edit_user");
			if(add_node){
				const char* attr_name = ezxml_attr(edit_node, "name");
				const char* attr_admin = ezxml_attr(edit_node, "admin");
				const char* attr_permit_live = ezxml_attr(edit_node, "permit_live");
				const char* attr_permit_setting = ezxml_attr(edit_node, "permit_setting");
				const char* attr_permit_playback = ezxml_attr(edit_node, "permit_playback");

				bool const is_admin = attr_admin ? (STR_CASE_THE_SAME(attr_admin, "yes")) : false;
				uint32_t permit_flag = 0; // clear flag

				if(attr_permit_live ? (STR_CASE_THE_SAME(attr_permit_live, "permit_live")) : false){
					permit_flag |= USRM_PERMIT_LIVE;
				}
				if(attr_permit_setting ? (STR_CASE_THE_SAME(attr_permit_setting, "permit_setting")) : false){
					permit_flag |= USRM_PERMIT_SETTING;
				}
				if(attr_permit_playback ? (STR_CASE_THE_SAME(attr_permit_playback, "permit_playback")) : false){
					permit_flag |= USRM_PERMIT_PLAYBACK;
				}

				how_about = i_m->edit_user(i_m, attr_name, is_admin, permit_flag);
				if(USRM_GREAT == how_about){
					add_success = true;
					APP_TRACE("Edit user \"%s\" success!", attr_name);
					USRM_store();
				}else{
					// FIXME:

					
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", AVAL_STRDUPA(av_username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	xml_text = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(http_session, xml_text);
	
	free(output_xml);
	output_xml = NULL;
	
	return 0;
}

int CGI_user_set_password(HTTPD_SESSION_t* http_session)
{
	int ret = 0;
	AVal av_username = AVC(""), av_password = AVC(""), av_content = AVC("");
	USRM_I_KNOW_U_t* i_m = NULL;
	bool checkin_success = false;
	bool set_success = false;
	ezxml_t output_xml = NULL;
	char* response_xml = NULL;
	
	char query_string[2048] = {""};
	
	// get current session username / password
	cgi_user_parse_query_string(http_session, query_string, &av_username, &av_password, &av_content);

	// user check in
	i_m = USRM_login(AVAL_STRDUPA(av_username), AVAL_STRDUPA(av_password));
	if(i_m){
		ezxml_t input_xml = NULL;
		checkin_success = true;

		APP_TRACE("Login success! Query string = \"%s\"", query_string);

		input_xml = ezxml_parse_str(av_content.av_val, av_content.av_len);
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t add_node = ezxml_child(input_xml, "set_pass");
			if(add_node){
				const char* attr_old_pass = ezxml_attr(add_node, "old_pass");
				const char* attr_new_pass = ezxml_attr(add_node, "new_pass");

				how_about = i_m->set_password(i_m, attr_old_pass, attr_new_pass);
				if(USRM_GREAT == how_about){
					set_success = true;
					APP_TRACE("Set user \"%s\" password success!", AVAL_STRDUPA(av_username));
					USRM_store();
				}else{
					// FIXME:
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", AVAL_STRDUPA(av_username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	response_xml = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(http_session, response_xml);
	
	free(output_xml);
	output_xml = NULL;
	
	return 0;
}


