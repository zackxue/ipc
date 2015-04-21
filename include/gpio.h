#ifndef __GPIO_H__
#define __GPIO_H__
#ifdef _cplusplus
extern "C" {
#endif


#define IRCUT_READ_PIN GPIO_read_bit("ircut")
#define IRCUT_WRITE_PIN(value) GPIO_write_bit("ircut",value)

extern unsigned char GPIO_read_bit(const char *name);
extern unsigned char GPIO_write_bit(const char *name, unsigned char value);
extern int GPIO_init();
extern void GPIO_destroy();


#ifdef _cplusplus
};
#endif
#endif //__GPIO_H__


