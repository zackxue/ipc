
#include "ddns.h"
#include "sysconf.h"


typedef struct DDNS
{
	DDNS_PARA_t ddns_para;
	DDNS_STAT_t ddns_stat;
	uint32_t ddns_trigger;
	pthread_t ddns_tid;
}DDNS_t;
static DDNS_t* _ddns = NULL;

void ddns_dyndns(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url, const char* username, const char* password);
void ddns_3322(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url, const char* username, const char* password);
void ddns_noip(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url, const char* username, const char* password);

static void* ddns_daemon(void* arg)
{
	// FIXME
	if(_ddns->ddns_para.provider==SYS_DDNS_PROVIDER_DYNDNS){
		ddns_dyndns(&_ddns->ddns_trigger, &_ddns->ddns_stat,
			_ddns->ddns_para.dyndns.register_url, _ddns->ddns_para.dyndns.username, _ddns->ddns_para.dyndns.password);
	}else if(_ddns->ddns_para.provider==SYS_DDNS_PROVIDER_NOIP){
		ddns_noip(&_ddns->ddns_trigger, &_ddns->ddns_stat,
			_ddns->ddns_para.noip.register_url, _ddns->ddns_para.noip.username, _ddns->ddns_para.noip.password);
	}else if(_ddns->ddns_para.provider==SYS_DDNS_PROVIDER_3322){
		ddns_3322(&_ddns->ddns_trigger, &_ddns->ddns_stat,
			_ddns->ddns_para._3322.register_url, _ddns->ddns_para._3322.username, _ddns->ddns_para._3322.password);	
	}else if(_ddns->ddns_para.provider==SYS_DDNS_PROVIDER_CHANGEIP){
		ddns_changeip(&_ddns->ddns_trigger, &_ddns->ddns_stat,
			_ddns->ddns_para.changeip.register_url, _ddns->ddns_para.changeip.username, _ddns->ddns_para.changeip.password);	
	}
	pthread_exit(NULL);
}

static void ddns_daemon_start()
{
	if(!_ddns->ddns_tid){
		int ret = 0;
		_ddns->ddns_trigger = true;
		ret = pthread_create(&_ddns->ddns_tid, NULL, ddns_daemon, NULL);
		assert(0 == ret);
	}
}

static void ddns_daemon_stop()
{
	if(_ddns->ddns_tid){
		_ddns->ddns_trigger = false;
		pthread_join(&_ddns->ddns_tid, NULL);
		_ddns->ddns_tid = (pthread_t)NULL;
	}
}

int DDNS_start(DDNS_PARA_t para)
{
	if(!_ddns){
		_ddns = calloc(sizeof(DDNS_t), 1);
		assert(_ddns);
		memcpy(&_ddns->ddns_para, &para, sizeof(para));
		_ddns->ddns_stat = DDNS_SUSPEND;
		_ddns->ddns_trigger = false;
		_ddns->ddns_tid = (pthread_t)NULL;
	}
	ddns_daemon_start();
	return 0;
}

int DDNS_restart(DDNS_PARA_t para)
{
	DDNS_quit();
	return DDNS_start(para);
}

void DDNS_quit()
{
	ddns_daemon_stop();
}

DDNS_STAT_t DDNS_get_stat()
{
	return DDNS_UNKNOW_ERROR;
}


