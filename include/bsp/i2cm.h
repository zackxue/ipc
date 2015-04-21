

#ifndef __I2CM_H__
#define __I2CM_H__
#ifdef _cplusplus
extern "C" {
#endif
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef __KERNEL__
#include <linux/types.h>
typedef u8 I2CM_U8_t;
typedef u16 I2CM_U16_t;
#else
#include <stdint.h>
typedef uint8_t I2CM_U8_t;
typedef uint16_t I2CM_U16_t;
#endif
	
typedef enum _I2C_PAYLOAD
{
	I2C_PT_BUS8 = (1<<0),
	I2C_PT_BUS16 = (1<<1),
	I2C_PT_SCCB = (1<<2),
	I2C_PT_DATA16 = (1<<3),
}I2C_PAYLOAD;

#pragma pack(1)
typedef struct _I2CPayloadBus8
{
	I2CM_U8_t dev;
	I2CM_U8_t addr;
	I2CM_U8_t val;
}I2CPayloadBus8;

typedef struct _I2CPayloadBus16
{
	I2CM_U8_t dev;
	I2CM_U16_t addr;
	I2CM_U8_t val;
}I2CPayloadBus16;

typedef struct _I2CPayloadSCCB
{
	I2CM_U8_t dev;
	I2CM_U8_t addr;
	I2CM_U8_t val;
}I2CPayloadSCCB;

typedef struct _I2CPayloadData16
{
	I2CM_U8_t dev;
	I2CM_U16_t addr;
	I2CM_U16_t val;
}I2CPayloadData16;
#pragma pack()

typedef struct _I2CPayload
{
	I2C_PAYLOAD method;
	union
	{
		I2CPayloadBus8 bus8;
		I2CPayloadBus16 bus16;
		I2CPayloadSCCB sccb;
		I2CPayloadData16 data16;
	};
}I2CPayload;

#define I2CM_DRV_MAGIC ('I')
//#define I2CM_DRV_WRITE _IOWR(I2CM_DRV_MAGIC, 1, I2CPayload)
//#define I2CM_DRV_READ _IOWR(I2CM_DRV_MAGIC, 2, I2CPayload)
#define I2CM_DRV_WRITE 1
#define I2CM_DRV_READ 0

#include "linux/version.h"

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,24) //rough judge
#include <asm/hardware.h>
#include <asm/semaphore.h>
#include <linux/smp_lock.h>
#ifdef MODULE
//#include <linux/compile.h>
#endif //MODULE
#endif


#ifdef  __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
#define ioctl unlocked_ioctl
#define DEFINE_IOCTL(k,x,y,z) long k (struct file *x, unsigned y, unsigned long z)
#else
#define DEFINE_IOCTL(k,x,y,z) int  k (struct inode *inode, \
                                      struct file *x, unsigned y, unsigned long z)
#endif


#include <asm/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>

#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/workqueue.h>
// external interface

#else
//////////////////////////////////////////////////////////////////////
// application
//////////////////////////////////////////////////////////////////////

extern int I2CM_init();
extern void I2CM_destroy();


extern int I2CM_write_bus8(I2CM_U8_t dev, I2CM_U8_t addr, I2CM_U8_t data);
extern I2CM_U8_t I2CM_read_bus8(I2CM_U8_t dev, I2CM_U8_t addr);

extern int I2CM_write_bus16(I2CM_U8_t dev, I2CM_U16_t addr, I2CM_U8_t val);
extern I2CM_U8_t I2CM_read_bus16(I2CM_U8_t dev, I2CM_U16_t val);

extern int I2CM_write_sccb(I2CM_U8_t dev, I2CM_U8_t addr, I2CM_U8_t val);
extern I2CM_U8_t I2CM_read_sccb(I2CM_U8_t dev, I2CM_U8_t addr);

extern int I2CM_write_data16(I2CM_U8_t dev, I2CM_U16_t addr, I2CM_U16_t val);
extern I2CM_U16_t I2CM_read_data16(I2CM_U8_t dev, I2CM_U16_t addr);

#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#ifdef _cplusplus
};
#endif
#endif //__I2CM_H__

