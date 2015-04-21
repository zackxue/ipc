
#include "inifile.h"
#include "md5sum.h"
#include "generic.h"
#include "usrm_debug.h"
#include "usrm.h"


typedef struct USER_HOURSE
{
	bool house_card;
	char username[32];
	char password[32];
	bool is_admin;
	//
	uint32_t permit_flag;
}USER_HOURSE_t;

struct USRM_WHO_AM_I
{
	USER_HOURSE_t* house;
};

// store step: memory -> ini -> storage
// load step: storage -> ini -> memory
typedef struct USR_MANAGER
{
	char version[16]; //
	// first user
	char first_username[32];
	char first_password[32];
	//
	char storage_md5[64]; // 32 bytes ascii for sum check
	char storage[32];
	//
	int n_user;
	USER_HOURSE_t user[USR_MANGER_USER_HOURSE_BACKLOG];
}USR_MANAGER_t;
static USR_MANAGER_t _usr_m =
{
		.storage = {""},
		//
		.n_user = 0,
};
static USR_MANAGER_t* _p_usr_m = NULL;

static int find_user_house(const char* what_name)
{
	if(_p_usr_m){
		int i = 0;
		for(i = 0; i < ARRAY_ITEM(_p_usr_m->user); ++i){
			USER_HOURSE_t* const user = _p_usr_m->user + i;
			if(user->house_card && STR_CASE_THE_SAME(what_name, user->username)){
				return i;
			}
		}
	}
	return -1;
}

USRM_HOW_ABOUT_t USRM_add_user(const char* what_name, const char* what_password, bool is_admin, uint32_t permit_flag, bool over_write)
{
	if(_p_usr_m && what_name){
		if(_p_usr_m->n_user < ARRAY_ITEM(_p_usr_m->user)){
			int i = 0;
			int user_house = find_user_house(what_name);
			if(user_house >= 0){
				if(over_write){
					// if over write
					// delete the old user first
					USRM_del_user(what_name);
				}else{
					USRM_TRACE("User \"%s\" has existed", what_name);
					return USRM_USER_EXIST;
				}
			}
			
			for(i = 0; i < ARRAY_ITEM(_p_usr_m->user); ++i){
				USER_HOURSE_t* const user_add = _p_usr_m->user + i;
				if(!user_add->house_card){
					if(strlen(what_name) < ARRAY_ITEM(user_add->username)){
						strcpy(user_add->username, what_name);
					}else{
						return USRM_USERNAME_TOO_LONG;
					}
					if(what_password){
						if(strlen(what_password) < ARRAY_ITEM(user_add->password)){
							strcpy(user_add->password, what_password);
						}else{
							return USRM_PASSWORD_TOO_LONG;
						}
					}else{
						// null password
						strcpy(user_add->password, "");
					}
					user_add->is_admin = is_admin;
					user_add->permit_flag = permit_flag;
					user_add->house_card = true;
					++_p_usr_m->n_user;
					USRM_TRACE("Add user \"%s\" @ %d", user_add->username, i);
					return USRM_GREAT;
				}
			}
		}else{
			USRM_TRACE("User full house %d", ARRAY_ITEM(_p_usr_m->user));
			return USEM_FULL_HOUSE;
		}
	}
	return USRM_INVALID;
}

USRM_HOW_ABOUT_t USRM_del_user(const char* what_name)
{
	if(what_name){
		int user_house = find_user_house(what_name);
		if(user_house >= 0){
			USER_HOURSE_t* const user_exist = _p_usr_m->user + user_house;
			user_exist->house_card = false; // release the house card
			--_p_usr_m->n_user;
			USRM_TRACE("Remove user \"%s\"", what_name);
			return USRM_GREAT;
		}else{
			// not user exist
			USRM_TRACE("User \"%s\" not existed", what_name);
			return USRM_USER_NO_EXIT;
		}
	}
	return USRM_INVALID;
}

USRM_HOW_ABOUT_t USRM_edit_user(const char* what_name, const char * what_password, bool is_admin, uint32_t permit_flag)
{
	int user_house = find_user_house(what_name);
	if(user_house >= 0){
		USER_HOURSE_t* const user_edit = _p_usr_m->user + user_house;
		if(what_password){
			if(strlen(what_password) < ARRAY_ITEM(user_edit->password)){
				strcpy(user_edit->password, what_password);
			}else{
				return USRM_PASSWORD_TOO_LONG;
			}
		}
		user_edit->is_admin = is_admin; // edit admin position
		user_edit->permit_flag = permit_flag; // edit access flag
		USRM_TRACE("Edit user \"%s\"", what_name);
		return USRM_GREAT;
	}
	return USRM_USER_NO_EXIT;
}

USRM_HOW_ABOUT_t USRM_check_user(const char* what_name, const char* what_password)
{
	int user_house = find_user_house(what_name);
	if(user_house >= 0){
		USER_HOURSE_t* const user_verify = _p_usr_m->user + user_house;
		if(STR_THE_SAME(what_password, user_verify->password)){
			return USRM_GREAT;
		}
		return USRM_PASSWORD_ERROR;
	}
	return USRM_USER_NO_EXIT;
}

USRM_HOW_ABOUT_t USRM_dump(const char* what_name)
{
	int user_house = find_user_house(what_name);
	if(user_house >= 0){
		USER_HOURSE_t* const user_dump = _p_usr_m->user + user_house;
		USRM_TRACE("User house %d\r\n\tUsername: %s\r\n\tPassword: %s\r\n\tAdmin: %s\r\n\tAccess: 0x%08x",
			user_house, user_dump->username, user_dump->password, user_dump->is_admin ? "yes" : "no", user_dump->permit_flag);
		return USRM_GREAT;
	}
	return USRM_USER_NO_EXIT;
}


//
// [OPTION]
// version=1.1
// n_user=1
// 
// [USER0]
// name=admin
// password=
// superuser=yes
//
// ...
//

static int memory_from_ini(const char* storage)
{
	bool read_success = false;
	// read the ini file from storage
	// then analyse the ini to memory
	if(_p_usr_m){
		FILE* fid = fopen(_p_usr_m->storage, "rb");
		if(fid){
			FILE* fid_ini = fopen(USR_MANGER_TMP_FILE, "w+b");
			if(fid_ini){
				int i = 0;
				int ret = 0;
				ssize_t write_size = 0;
				ssize_t read_size = 0;
				char buf[1024];
				lpINI_PARSER ini = NULL;
				
				// read md5
				ret = fread(_p_usr_m->storage_md5, 1, ARRAY_ITEM(_p_usr_m->storage_md5), fid);
				USRM_ASSERT(ARRAY_ITEM(_p_usr_m->storage_md5) == ret, "Read MD5 %d/%d",
						ret, ARRAY_ITEM(_p_usr_m->storage_md5));
				USRM_TRACE("MD5 \"%s\"", _p_usr_m->storage_md5);

				// read ini file
				while((read_size = fread(buf, 1, sizeof(buf), fid)) > 0){
					write_size  = fwrite(buf, 1, read_size, fid_ini);
					USRM_ASSERT(write_size == read_size, "Write INI %d/%d",
						write_size, read_size);
				}
				fclose(fid_ini);
				fid_ini = NULL;

				// read ini file and write to memory
				ini = OpenIniFile(USR_MANGER_TMP_FILE);
				// read version
				strcpy(_p_usr_m->version, ini->read_text(ini, "OPTION", "version", "", buf, sizeof(buf)));
				// read users
				_p_usr_m->n_user = ini->read_int(ini, "OPTION", "user", 0);
				for(i = 0; i < _p_usr_m->n_user; ++i){
					char user_section[32];
					USER_HOURSE_t* const user = _p_usr_m->user + i;
					sprintf(user_section, "USER%d", i);

					user->house_card = true; // very important
					strcpy(user->username, ini->read_text(ini, user_section, "name", "", buf, sizeof(buf)));
					strcpy(user->password, ini->read_text(ini, user_section, "password", "", buf, sizeof(buf)));
					user->is_admin = ini->read_bool(ini, user_section, "admin", "");
					user->permit_flag = 0;
					user->permit_flag |= ini->read_bool(ini, user_section, "permit_live", false) ? USRM_PERMIT_LIVE : 0;
					user->permit_flag |= ini->read_bool(ini, user_section, "permit_setting", false) ? USRM_PERMIT_SETTING : 0;
					user->permit_flag |= ini->read_bool(ini, user_section, "permit_playback", false) ? USRM_PERMIT_PLAYBACK : 0;
					
				}
				CloseIniFile(ini);

				read_success = true;
			}
			fclose(fid);
			fid = NULL;
		}
	}
	return read_success ? 0 : -1;
}

static void memory_to_ini()
{
	if(_p_usr_m){
		FILE* fid = NULL;
		lpINI_PARSER ini = NULL;
		int i = 0;
		int user_counter = 0;
		
		// clear the temp file
		fid = fopen(USR_MANGER_TMP_FILE, "w+b");
		fclose(fid);
		fid = NULL;

		// start to write ini file
		// write option
		ini = OpenIniFile(USR_MANGER_TMP_FILE);
		ini->write_text(ini, "OPTION", "version", USR_MANGER_VERSION);
		ini->write_int(ini, "OPTION", "user", _p_usr_m->n_user);

		for(i = 0; i < ARRAY_ITEM(_p_usr_m->user); ++i){
			USER_HOURSE_t* const user = _p_usr_m->user + i;
			if(user->house_card){
				char user_section[64] = {""};
				
				sprintf(user_section, "USER%d", user_counter++);
				//USRM_TRACE("Section [%s]", user_section);
				ini->write_text(ini, user_section, "name", user->username);
				ini->write_text(ini, user_section, "password", user->password);
				ini->write_bool(ini, user_section, "admin", user->is_admin);
				ini->write_bool(ini, user_section, "permit_live", (user->permit_flag & USRM_PERMIT_LIVE) ? true : false);
				ini->write_bool(ini, user_section, "permit_setting", (user->permit_flag & USRM_PERMIT_SETTING) ? true : false);
				ini->write_bool(ini, user_section, "permit_playback", (user->permit_flag & USRM_PERMIT_PLAYBACK) ? true : false);
			}
			
		}
		WriteIniFile(ini, USR_MANGER_TMP_FILE);
		CloseIniFile(ini);
		ini = NULL;
	}
}

static void storage_reset()
{
	if(_p_usr_m){
		_p_usr_m->n_user = 0;
		ARRAY_ZERO(_p_usr_m->user);
		
		// add the 1st user name:admin
		//USRM_add_user(_p_usr_m->first_username, _p_usr_m->first_password, true, USRM_PERMIT_ALL, true);

		// testing account
		USRM_add_user("admin", "", true, USRM_PERMIT_ALL, true);
		USRM_add_user("user", "user", true, USRM_PERMIT_ALL, true);
		USRM_add_user("guest", "", false, USRM_PERMIT_ALL, true);
		
		USRM_store();
	}
}

static bool storage_check()
{
	if(STR_THE_SAME(_p_usr_m->version, USR_MANGER_VERSION)){
		const char* md5_file = md5sum_file(USR_MANGER_TMP_FILE);
		if(0 == strncasecmp(md5_file, _p_usr_m->storage_md5, strlen(md5_file))){
			return true;
		}
		USRM_TRACE("MD5 check failed!\r\n%s\r\n%s", md5_file, _p_usr_m->storage_md5);
	}
	return false;
}

static int storage_load(const char* storage)
{
	memory_from_ini(storage);
	return storage_check() ? 0 : -1;
}

static int storage_store(const char* storage)
{
	ssize_t storage_size = 0;
	char* buf = NULL;
	const char* md5_text = NULL;
	ssize_t const md5_size = ARRAY_ITEM(_p_usr_m->storage_md5);
	
	//GET_FILE_SIZE(storage, storage_size);
	FILE* fid = fopen(storage, "r+b");
	if(fid){
		fseek(fid, 0, SEEK_END);
		storage_size = ftell(fid);
		fclose(fid);

		if(0 == storage){
			GET_FILE_SIZE(USR_MANGER_TMP_FILE, storage_size);
			storage_size += md5_size;
		}

		// load ini file to memory
		buf = alloca(storage_size);
		memset(buf, 0, storage_size);
		fid = fopen(USR_MANGER_TMP_FILE, "r+b");
		fread(buf + md5_size, 1, storage_size - md5_size, fid);
		fclose(fid);
		fid = NULL;
		

		// sum memory md5 
		md5_text = md5sum_buffer(buf + md5_size, storage_size - md5_size);
		strcpy(buf, md5_text);

		fid = fopen(storage, "w+b");
		fwrite(buf, 1, storage_size, fid);
		fclose(fid);
		return 0;
	}

//	unlink(USR_MANGER_TMP_FILE);
//	remove(USR_MANGER_TMP_FILE);
	
	//USRM_TRACE("Storage size = %d", storage_size);
	return -1;
}

int USRM_init(const char* storage, const char* first_username, const char* first_password)
{
	if(!_p_usr_m){
		// init the pointer
		STRUCT_ZERO(_usr_m);
		_p_usr_m = &_usr_m;

		// first user name / password
		if(first_username){
			strncpy(_p_usr_m->first_username, first_username, ARRAY_ITEM(_p_usr_m->first_username));
			if(first_password){
				strncpy(_p_usr_m->first_password, first_password, ARRAY_ITEM(_p_usr_m->first_password));
			}else{
				strcpy(_p_usr_m->first_password, "");
			}
		}
		//
		strncpy(_p_usr_m->storage, storage, ARRAY_ITEM(_p_usr_m->storage));
		
		// load user info to memory
		if(0 != storage_load(_p_usr_m->storage)){
			storage_reset();
		}
		return 0;
	}
	return -1;
}

void USRM_destroy()
{
	if(_p_usr_m){
		_p_usr_m = NULL;
	}
}

int USRM_store()
{
	memory_to_ini();
	return storage_store(_p_usr_m->storage);
}



static USRM_HOW_ABOUT_t usrm_add_user(struct USRM_I_KNOW_U* const i_am, const char* what_name, const char* what_password, bool is_admin, uint32_t permit_flag)
{
	struct USRM_WHO_AM_I* const who_am_i = (struct USRM_WHO_AM_I*)(i_am + 1);
	// only admin could do this
	if(who_am_i->house->house_card){
		if(who_am_i->house->is_admin){
			return USRM_add_user(what_name, what_password, is_admin, permit_flag, false);
		}
		return USRM_ADMIN_ONLY;
	}
	return USRM_NEVER_LOGIN;
}

static USRM_HOW_ABOUT_t usrm_del_user(struct USRM_I_KNOW_U* const i_am, const char* what_name)
{
	struct USRM_WHO_AM_I* const who_am_i = (struct USRM_WHO_AM_I*)(i_am + 1);
	// check whether i want to delete myself
	if(who_am_i->house->house_card){
		if(!STR_CASE_THE_SAME(who_am_i->house->username, what_name)){
			if(who_am_i->house->is_admin){
				return USRM_del_user(what_name);
			}
			// only admin could do this
			return USRM_ADMIN_ONLY;
		}
		return USRM_NOT_PERMIT; // not permit to delete myself
	}
	return USRM_NEVER_LOGIN;
}

static USRM_HOW_ABOUT_t usrm_edit_user(struct USRM_I_KNOW_U* const i_am, const char* what_name, bool is_admin, uint32_t permit_flag)
{
	struct USRM_WHO_AM_I* const who_am_i = (struct USRM_WHO_AM_I*)(i_am + 1);
	// only admin could do this
	if(who_am_i->house->house_card){
		// check whether i want to edit myself
		if(!STR_CASE_THE_SAME(who_am_i->house->username, what_name)){
			if(who_am_i->house->is_admin){
				return USRM_edit_user(what_name, NULL, is_admin, permit_flag);
			}
			// only admin could do this
			return USRM_ADMIN_ONLY;
		}
		return USRM_NOT_PERMIT; // not permit to edit myself
	}
	return USRM_NEVER_LOGIN;
}

static USRM_HOW_ABOUT_t usrm_set_password(struct USRM_I_KNOW_U* const i_am, const char* what_old_password, const char* what_new_password)
{
	struct USRM_WHO_AM_I* const who_am_i = (struct USRM_WHO_AM_I*)(i_am + 1);
	if(who_am_i->house->house_card){
		// check whether i operate by myself
		USRM_HOW_ABOUT_t how_about = USRM_check_user(who_am_i->house->username, what_old_password);
		if(USRM_GREAT == how_about){
			if(strlen(what_new_password) < ARRAY_ITEM(who_am_i->house->password)){
				strcpy(who_am_i->house->password, what_new_password);
				return USRM_GREAT;
			}else{
				return USRM_PASSWORD_TOO_LONG;
			}
		}
		return how_about;
	}
	return USRM_NEVER_LOGIN;
}

static USRM_HOW_ABOUT_t usrm_dump(struct USRM_I_KNOW_U* const i_am)
{
	struct USRM_WHO_AM_I* const who_am_i = (struct USRM_WHO_AM_I*)(i_am + 1);
	if(who_am_i->house->house_card){
		// only dump myself
		return USRM_dump(who_am_i->house->username);
	}
	return USRM_NEVER_LOGIN;
}

USRM_I_KNOW_U_t* USRM_login(const char* who_r_u, const char* what_s_password)
{
	int i = 0;
	if(_p_usr_m){
		for(i = 0; i < ARRAY_ITEM(_p_usr_m->user); ++i){
			USER_HOURSE_t* const user_house = _p_usr_m->user + i;
			if(user_house->house_card){
				if(STR_CASE_THE_SAME(who_r_u, user_house->username)
					&& STR_THE_SAME(what_s_password, user_house->password)){
					// matching this user
					USRM_I_KNOW_U_t* const i_know_u = calloc(sizeof(USRM_I_KNOW_U_t) + sizeof(struct USRM_WHO_AM_I), 1);
					struct USRM_WHO_AM_I* const who_am_i = (struct USRM_WHO_AM_I*)(i_know_u + 1);

					i_know_u->add_user = usrm_add_user;
					i_know_u->del_user = usrm_del_user;
					i_know_u->edit_user = usrm_edit_user;
					i_know_u->set_password = usrm_set_password;
					i_know_u->dump = usrm_dump;

					// know which is my house
					who_am_i->house = user_house;

					// public attribute
					i_know_u->forbidden_zero = 0;
					i_know_u->username = who_am_i->house->username;
					i_know_u->password = who_am_i->house->password;
					i_know_u->is_admin = who_am_i->house->is_admin;
					i_know_u->permit_flag = who_am_i->house->permit_flag;

					// return this pointer tell him that you know him
					return i_know_u;
				}
			}
		}
	}
	return NULL;
}

void USRM_logout(USRM_I_KNOW_U_t* i_know_u)
{
	free(i_know_u);
}

/*
int main()
{
	USRM_init("./mtdblock2");

	USRM_I_KNOW_U_t* i_m = USRM_login("admin", "");
	if(i_m){
		USRM_TRACE("Login success!");
		i_m->dump(i_m);
		i_m->set_password(i_m, "", "123443");
		
		
		USRM_logout(i_m);
	}

	USRM_store();
	return 0;
}
*/


