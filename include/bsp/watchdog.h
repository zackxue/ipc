
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __KERNEL__
// driver
//#include <asm/hardware/clock.h>
//#include <asm/arch/clock.h>
#include <asm/uaccess.h>
#include <asm/io.h>
//#include <asm/arch/hardware.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
//#include <linux/config.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/sched.h>

#include "linux/version.h"

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,24) //rough judge
#include <asm/hardware.h>
#include <asm/semaphore.h>
#include <linux/smp_lock.h>
#ifdef MODULE
//#include <linux/compile.h>
#endif //MODULE
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
#define ioctl unlocked_ioctl
#define DEFINE_IOCTL(k,x,y,z) long k (struct file *x, unsigned y, unsigned long z)
#else
#define DEFINE_IOCTL(k,x,y,z) int  k (struct inode *inode, \
                                      struct file *x, unsigned y, unsigned long z)
#endif



#else //__KERNEL__
// application
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <linux/watchdog.h> // must be after than sys/ioctl.h

extern int WATCHDOG_init(int timeout_s);
extern void WATCHDOG_destroy();

extern int WATCHDOG_enable();
extern int WATCHDOG_disable();

extern int WATCHDOG_get_timeout();
extern int WATCHDOG_set_timeout(int timeout_s);

extern int WATCHDOG_feed();

#endif //else __KERNEL__

#ifdef __cplusplus
};
#endif
#endif //__WATCHDOG_H__

