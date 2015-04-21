
#include "app_gpio.h"
#include "sdk/sdk_api.h"
#include "app_debug.h"

#define kAPP_GPIO_ENTRY_AVAIALBE (32) 

typedef struct APP_GPIO_ENTRY {
	char name[16];
	uint32_t dir_addr32, dir_mask32, dir_in_val32, dir_out_val32;
	uint32_t data_addr32, data_mask32;
}stAPP_GPIO_ENTRY, *lpAPP_GPIO_ENTRY;

typedef struct APP_GPIO {
	stAPP_GPIO_ENTRY entry[kAPP_GPIO_ENTRY_AVAIALBE];
}stAPP_GPIO, *lpAPP_GPIO;

static stAPP_GPIO _app_gpio = {
};

static int app_gpio_lookup_byname(const char *name)
{
	int i = 0;
	for(i = 0; i < kAPP_GPIO_ENTRY_AVAIALBE; ++i){
		lpAPP_GPIO_ENTRY const gpio_entry = &_app_gpio.entry[i];
		if(0 == strcasecmp(name, gpio_entry->name)
			&& strlen(name) == strlen(gpio_entry->name)){
			return i;
		}
	}
	return -1;
}

int APP_GPIO_add(const char *name, uint32_t conf_addr32, uint32_t conf_mask32, uint32_t conf_val32,
	uint32_t dir_addr32, uint32_t dir_mask32, uint32_t dir_in_val32, uint32_t dir_out_val32,
	uint32_t data_addr32, uint32_t data_mask32)
{
	int id = 0;
	
	if(NULL != name && strlen(name) > 0){
		id = app_gpio_lookup_byname(name);
		if(id >= 0){
			// a existed name found
			APP_TRACE("GPIO \"%s\" has existed!", name);
			return -1;
		}
		// lookup a position
		id = app_gpio_lookup_byname("");
		if(id >= 0){
			lpAPP_GPIO_ENTRY const gpio_entry = &_app_gpio.entry[id];
			if(0 == conf_addr32 // ignore the configurate
				|| (NULL != sdk_sys && 0 == sdk_sys->write_mask_reg(conf_addr32, conf_mask32, conf_val32))){
				// add a new one
				strcpy(gpio_entry->name, name);
				gpio_entry->dir_addr32 = dir_addr32;
				gpio_entry->dir_mask32 = dir_mask32;
				gpio_entry->dir_in_val32 = dir_in_val32;
				gpio_entry->dir_out_val32 = dir_out_val32;
				gpio_entry->data_addr32 = data_addr32;
				gpio_entry->data_mask32 = data_mask32;
				return 0;
			}
			APP_TRACE("GPIO \"%s\" conf(%08x) failed", name, conf_addr32);
			return -1;
		}
	}
	APP_TRACE("GPIO too many");
	return 1;
}

int APP_GPIO_del(const char *name)
{
	int id = 0;
	if(NULL != name && strlen(name) > 0){
		id = app_gpio_lookup_byname(name);
		if(id >= 0){
			lpAPP_GPIO_ENTRY const gpio_entry = &_app_gpio.entry[id];
			// delete this gpio entry
			gpio_entry->name[0] = '\0';
			return 0;
		}
	}
	return -1;
}

int APP_GPIO_set_pin(const char *name, bool pin_high)
{
	if(NULL != name && strlen(name) > 0){
		int const id = app_gpio_lookup_byname(name);
		lpAPP_GPIO_ENTRY const gpio_entry = &_app_gpio.entry[id];
		if(NULL != sdk_sys){
			// set direction
			if(0 == sdk_sys->write_mask_reg(gpio_entry->dir_addr32, gpio_entry->dir_mask32, gpio_entry->dir_out_val32)){
				// set data
				if(0 == sdk_sys->write_mask_reg(gpio_entry->data_addr32, gpio_entry->data_mask32, pin_high ? 0xffffffff : 0)){
					return 0;
				}
			}
		}
	}
	return -1;
}

int APP_GPIO_get_pin(const char *name, bool *pin_high)
{
	if(NULL != name && strlen(name) > 0){
		int const id = app_gpio_lookup_byname(name);
		lpAPP_GPIO_ENTRY const gpio_entry = &_app_gpio.entry[id];
		if(NULL != sdk_sys){
			// set direction
			if(0 == sdk_sys->write_mask_reg(gpio_entry->dir_addr32, gpio_entry->dir_mask32, gpio_entry->dir_in_val32)){
				// get data
				uint32_t val32_r = 0;
				if(0 == sdk_sys->read_mask_reg(gpio_entry->data_addr32, gpio_entry->data_mask32, &val32_r)){
					if(pin_high){
						*pin_high = val32_r ? true : false;
						return 0;
					}
				}
			}
		}
	}
	return -1;
}
	
static void app_gpio_cleanup()
{
	int i = 0;
	for(i = 0; i < kAPP_GPIO_ENTRY_AVAIALBE; ++i){
		lpAPP_GPIO_ENTRY const gpio_entry = &_app_gpio.entry[i];
		gpio_entry->name[0] = '\0';
	}
}

int APP_GPIO_init()
{
	app_gpio_cleanup();
	// FIXEME:
	return 0;
}

void APP_GPIO_destroy()
{
	app_gpio_cleanup();
}

