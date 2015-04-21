
#include "bsp/watchdog.h"

#ifdef __KERNEL__

#define WATCHDOG_TRACE(fmt...) \
    do{printk("\033[1;31mWATCHDOG->[%s]:%d ", __FUNCTION__, __LINE__);printk(fmt);printk("\033[m\r\n");}while(0)

#define HIWATCHDOG_BASE	(0x20040000)
#define HISILICON_SCTL_BASE (0x20050000)
#define HIWATCHDOG_REG(x) (HIWATCHDOG_BASE + (x))

#define HIWATCHDOG_READL(x) readl(IO_ADDRESS(HIWATCHDOG_REG(x)))
#define HIWATCHDOG_WRITEL(v,x) writel(v, IO_ADDRESS(HIWATCHDOG_REG(x)))

#define WATCHDOG_VERSION_INT (100)
#define WATCHDOG_VERSION_STR "100"
#define WATCHDOG_SUPPORT_OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING)
#define WATCHDOG_DEFAULT_TIMEOUT (10)
//#define MHZ	(1000000UL)
#define WATCHDOG_CLK_USE_3M	//if no define , use APB clock (135M)
#ifdef WATCHDOG_CLK_USE_3M
	#define WATCHDOG_CLK (3*MHZ)
	#define WATCHDOG_MAX_TIMOUT	(0xFFFFFFFF/WATCHDOG_CLK)	//0xffffffff/3M=1431s
#else
	#define APB_CLOCK	135
	#define WATCHDOG_CLK (APB_CLOCK*MHZ)
	#define WATCHDOG_MAX_TIMOUT (0xFFFFFFFF/WATCHDOG_CLK)	//0xffffffff/135M=31s
#endif

#define HIWDT_LOAD	0x000
#define HIWDT_VALUE	0x004
#define HIWDT_CTRL	0x008
#define HIWDT_INTCLR	0x00C
#define HIWDT_RIS	0x010
#define HIWDT_MIS	0x014
#define HIWDT_LOCK	0xC00

#define HIWDT_UNLOCK_VAL	0x1ACCE551

#ifndef MHZ
#define MHZ (1000*1000)
#endif


static DEFINE_SPINLOCK(hidog_lock);

static void _watchdog_settimeout(int timeout_s)
{
	unsigned long timeout;
	unsigned long flags; 
	if(!timeout_s){
		timeout = WATCHDOG_DEFAULT_TIMEOUT*WATCHDOG_CLK;
	}
	else if(timeout_s>WATCHDOG_MAX_TIMOUT){
		timeout = WATCHDOG_DEFAULT_TIMEOUT*WATCHDOG_CLK;	
	}
	else{
		timeout=timeout_s*WATCHDOG_CLK;
	}
	
	spin_lock_irqsave(&hidog_lock, flags); 
	
	HIWATCHDOG_WRITEL(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
	HIWATCHDOG_WRITEL(timeout, HIWDT_LOAD); 
	HIWATCHDOG_WRITEL(timeout, HIWDT_VALUE); 
	HIWATCHDOG_WRITEL(0, HIWDT_LOCK); 
	
	spin_unlock_irqrestore(&hidog_lock, flags); 
}

static unsigned int _watchdog_gettimeout(void)
{
	unsigned long flags;
	unsigned long timeout=0;
	spin_lock_irqsave(&hidog_lock, flags); 

	timeout= HIWATCHDOG_READL(HIWDT_LOAD)/WATCHDOG_CLK;
	
	spin_unlock_irqrestore(&hidog_lock, flags); 
	return timeout;
	
}

static void _watchdog_disablecard(void)
{
	/* unlock watchdog registers */
	HIWATCHDOG_WRITEL(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
	/* stop watchdog timer */
	HIWATCHDOG_WRITEL(0x00, HIWDT_CTRL);
	HIWATCHDOG_WRITEL(0x00, HIWDT_INTCLR);
	/* lock watchdog registers */
	HIWATCHDOG_WRITEL(0, HIWDT_LOCK);
}

static void _watchdog_enablecard(void)
{
	/* unlock watchdog registers */
	unsigned long flags; 
	unsigned long t;
	spin_lock_irqsave(&hidog_lock, flags); 
	HIWATCHDOG_WRITEL(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
	HIWATCHDOG_WRITEL(0x00, HIWDT_CTRL);
	HIWATCHDOG_WRITEL(0x00, HIWDT_INTCLR);
	HIWATCHDOG_WRITEL(0x03, HIWDT_CTRL);
	/* lock watchdog registers */
	HIWATCHDOG_WRITEL(0, HIWDT_LOCK);
	/* enable watchdog clock */
	t = readl(IO_ADDRESS(HISILICON_SCTL_BASE));
	#ifdef WATCHDOG_CLK_USE_3M
		writel(t & ~0x00800000, IO_ADDRESS(HISILICON_SCTL_BASE));	 //use 3MHz
	#else
		writel(t | 0x00800000, IO_ADDRESS(HISILICON_SCTL_BASE));	
	#endif
	
	spin_unlock_irqrestore(&hidog_lock, flags); 
}

static void _watchdog_keepalive(void)
{
	unsigned long flags; 
	spin_lock_irqsave(&hidog_lock, flags); 

	/* unlock watchdog registers */ 
	HIWATCHDOG_WRITEL(HIWDT_UNLOCK_VAL, HIWDT_LOCK); 
	/* clear watchdog */ 
	HIWATCHDOG_WRITEL(0x00, HIWDT_INTCLR); 
	/* lock watchdog registers */ 
	HIWATCHDOG_WRITEL(0, HIWDT_LOCK); 
	spin_unlock_irqrestore(&hidog_lock, flags); 
}

static int watchdog_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int watchdog_release(struct inode *inode, struct file *file)
{
	return 0;
}

static DEFINE_IOCTL(watchdog_ioctl, file, cmd, arg)
//static int watchdog_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;

	switch(cmd)
	{
	case WDIOC_GETSUPPORT:
		// return options
		{
			struct watchdog_info info = {0};
			memset(&info, 0, sizeof(info));
			info.options = WATCHDOG_SUPPORT_OPTIONS;
			info.firmware_version = WATCHDOG_VERSION_INT;
			strcpy(info.identity, "hi3518_ipcam");
			copy_to_user(argp, &info, sizeof(info)) ? -EFAULT : 0;
			return 0;
		}
		break;

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
	case WDIOC_GETTEMP:
		break;
		
	case WDIOC_SETOPTIONS:
		{
			// TODO
			int status = 0;
			ret = copy_from_user(&status, argp, sizeof(status));
			if(ret){
				return ret;
			}
			if(WDIOS_DISABLECARD == status){
				_watchdog_disablecard();
				return 0;
			}else if(WDIOS_ENABLECARD == status){
				_watchdog_enablecard();
				return 0;
			}else if(WDIOS_TEMPPANIC == status){
			}
			return WDIOS_UNKNOWN;
		}
		break;
		
	case WDIOC_KEEPALIVE:
		{
			_watchdog_keepalive();
			return 0;
		}
		break;

	case WDIOC_SETTIMEOUT:
		{
			int timeout_s = 0;
			ret = copy_from_user(&timeout_s, argp, sizeof(timeout_s));
			_watchdog_settimeout(timeout_s);
			return 0;
		}
		break;

	case WDIOC_GETTIMEOUT:
		{
			int timeout_s = _watchdog_gettimeout();
			copy_to_user(argp, &timeout_s, sizeof(timeout_s)) ? -EFAULT : 0;
			return 0;
		}
		break;

//	case WDIOC_SETPRETIMEOUT:
//	case WDIOC_GETPRETIMEOUT:
//	case WDIOC_GETTIMELEFT:
//		break;

	default:
		;
	}
	return -ENOIOCTLCMD;
}

static struct file_operations watchdog_fops =
{
	.owner = THIS_MODULE,
	.ioctl = watchdog_ioctl,
	.open = watchdog_open,
	.release	= watchdog_release,
};

static struct miscdevice watchdog_miscdev = {
	.minor = WATCHDOG_MINOR,
	.name = "watchdog",
	.fops = &watchdog_fops,
};

static int __init watchdog_init(void)
{
	int ret = 0;

	ret = misc_register(&watchdog_miscdev);
	if(0 == ret){
		WATCHDOG_TRACE("Watchdog register succeed!");
		return ret;
	}
	return ret;
}

static void __exit watchdog_exit(void)
{
	_watchdog_disablecard();
	misc_deregister(&watchdog_miscdev);
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Frank Law");
MODULE_DESCRIPTION("Guangzhou JUAN Watchdog Device Driver");
MODULE_LICENSE("LGPL");
MODULE_VERSION("v" WATCHDOG_VERSION_STR);


#else //__KERNEL__

#define WATCHDOG_TRACE(fmt...) \
    do{printf("\033[1;31mWATCHDOG->[%s]:%d ", __FUNCTION__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)

#define WATCHDOG_DEV "/dev/watchdog"

typedef struct WatchDog
{
	uint32_t autofeed_trigger;
	pthread_t autofeed_tid;

	int timeout_s;
	int fid;
	
}WatchDog_t;
static WatchDog_t* _watchdog = NULL;

int WATCHDOG_init(int timeout_s)
{
	if(!_watchdog){
		_watchdog = calloc(sizeof(WatchDog_t), 1);
		assert(_watchdog);
		// init
		_watchdog->autofeed_tid = (pthread_t)NULL;
		_watchdog->autofeed_trigger = false;
		_watchdog->timeout_s = timeout_s;
		_watchdog->fid = open(WATCHDOG_DEV, O_RDWR);
		if(_watchdog->fid < 0){
			_watchdog->fid = 0;
		}
		WATCHDOG_set_timeout(timeout_s);
		WATCHDOG_TRACE("get watchdog timeout:%lu \r\n", WATCHDOG_get_timeout());
		WATCHDOG_enable();
		return 0;
	}
	return -1;
}

void WATCHDOG_destroy()
{
	if(_watchdog){
		WATCHDOG_disable();
		if(_watchdog->fid > 0){
			close(_watchdog->fid);
			_watchdog->fid = 0;
		}
		free(_watchdog);
		_watchdog = NULL;
	}
}

static int watchdog_enable(int flag)
{
	int ret = 0;
	int status = flag ? WDIOS_ENABLECARD : WDIOS_DISABLECARD;
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_SETOPTIONS, &status);
		if(0 == ret){	
			return 0;
		}
	}
	WATCHDOG_TRACE("%s Failed!", flag ? "Enable" : "Disable");
	return -1;
}


int WATCHDOG_enable()
{
	return watchdog_enable(true);
}

int WATCHDOG_disable()
{
	return watchdog_enable(false);
}

int WATCHDOG_get_timeout()
{
	int ret = 0;
	int timeout_s = 0;
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_GETTIMEOUT, &timeout_s);
		if(0 == ret){	
			return timeout_s;
		}
	}
	WATCHDOG_TRACE("Get Timeout Failed!");
	return -1;
}

// 1 - 1431 second, 0 driver default
int WATCHDOG_set_timeout(int timeout_s)
{
	int ret = 0;
	if(!(timeout_s >= 0 && timeout_s <= 2862)){
		WATCHDOG_TRACE("Timeout Out of Range!");
		return -1;
	}
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_SETTIMEOUT, &timeout_s);
		if(0 == ret){
			_watchdog->timeout_s = WATCHDOG_get_timeout();
			return 0;
		}
	}
	WATCHDOG_TRACE("Set Timeout Failed!");
	return 0;
}

static int watchdog_keepalive()
{
	int ret = 0;
	int arg = 0;
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_KEEPALIVE, &arg);
		if(0 == ret){
			return 0;
		}
	}
//	WATCHDOG_TRACE("Keepalive Failed!");
	return -1;
}

int WATCHDOG_feed()
{
	return watchdog_keepalive();
}

#endif //__KERNEL__

