
#include "bsp/i2cm.h"

#ifdef __KERNEL__

#include <linux/bcd.h>
#include <linux/poll.h>
#include <linux/rtc.h>


#ifndef BCD2BIN
#define BCD2BIN bcd2bin
#define BIN2BCD bin2bcd
#endif

//#define RTC_DEBUG


static int rtc_read_proc(char *page, char **start, off_t off,
                         int count, int *eof, void *data);

/*
 *	Bits in rtc_status. (6 bits of room for future expansion)
 */

#define RTC_IS_OPEN		0x01	/* means /dev/rtc is in use	*/
#define RTC_TIMER_ON	0x02	/* missed irq timer active	*/

static unsigned char rtc_status;	/* bitmapped status byte.	*/
static unsigned long rtc_freq;	/* Current periodic IRQ rate	*/


/*
 *	If this driver ever becomes modularised, it will be really nice
 *	to make the epoch retain its value across module reload...
 */

static unsigned long epoch = 1970;	/* year corresponding to 0x00	*/

static const unsigned char days_in_mo[] =
{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define PCF8563_ADDR 0xA2

//#define RTC_DEBUG


//added by chendh 20060822
#define SECONDS 		0x00
#define MINUTES 		0x10
#define HOURS 			0x20
#define WEEKDAYS		0x30
#define DAYS 			0x40
#define MONTHS 			0x50
#define YEARS 			0x60

#define CONTROL1 		0xE0



inline void pcf8563_i2c_read(unsigned char Addr, unsigned char Buf[], int Size)
{
	int i=0;
	unsigned char ret;
	for(i=0;i<Size;i++)
	{
	
		extern I2CM_U8 i2cm_read_bus8(I2CM_U8 dev, I2CM_U8 addr);
    	ret=i2cm_read_bus8(PCF8563_ADDR, Addr);
		#ifdef RTC_DEBUG
		printk("[%02x(%d)]",ret,ret);
		#endif
		Buf[i]=ret;
	}
}

inline void pcf8563_i2c_write(unsigned char Addr, unsigned char Buf[], int Size)
{
	int i=0;
	for(i=0;i<Size;i++)
	{
		extern int i2cm_write_bus8(I2CM_U8 dev, I2CM_U8 addr, I2CM_U8 val);
		i2cm_write_bus8(PCF8563_ADDR,Addr+i,Buf[i]);
	}
}

static void pcf8563_init(void)
{
	unsigned char buf[4];
	
	pcf8563_i2c_read(0x0E, buf, 1);
	if(0x02 != (0x03 & buf[0])){
		buf[0] = 0x02;
		pcf8563_i2c_write(0x0E, buf, 1);
		pcf8563_i2c_read(0x02, buf, 1);
		printk("PCF8563 first init!\n");
	}
	
	buf[0] = 0;
	buf[1] = 0;
	pcf8563_i2c_write(0, buf, 2);

#if 0	//for test
{
	struct rtc_time rtc_tm;
	unsigned char buf[8];

	rtc_tm.tm_sec = 0x00;
	rtc_tm.tm_min = 0x12;
	rtc_tm.tm_hour = 0x14;
	rtc_tm.tm_mday = 0x15;
	rtc_tm.tm_mon = 0x08;
	rtc_tm.tm_year = 0x08;
	rtc_tm.tm_wday = 4;
	
	buf[0] = 	rtc_tm.tm_sec  ;
	buf[1] = 	rtc_tm.tm_min  ;
	buf[2] = 	rtc_tm.tm_hour ;
  buf[3] = 	rtc_tm.tm_mday ;
  buf[4] = 	rtc_tm.tm_wday ;
  buf[5] = 	rtc_tm.tm_mon  ;
  buf[6] = 	rtc_tm.tm_year ;
  
  pcf8563_i2c_write(2, buf, 7);
  
  while(1){
  	mdelay(1000);
  	memset(buf, 0x00, 7);
  	
  	#if 0
  	pcf8563_i2c_read(2, buf, 7);
  	#else
  	pcf8563_i2c_read(2, buf, 1);
  	pcf8563_i2c_read(3, &buf[1], 1);
  	pcf8563_i2c_read(4, &buf[2], 1);
  	pcf8563_i2c_read(5, &buf[3], 1);
  	pcf8563_i2c_read(6, &buf[4], 1);
  	pcf8563_i2c_read(7, &buf[5], 1);
  	pcf8563_i2c_read(8, &buf[6], 1);
  	#endif
  	
  	buf[0] = buf[0] & 0x7F;
	  buf[1] = buf[1] & 0x7F;
	  buf[2] = buf[2] & 0x3F;
	  buf[3] = buf[3] & 0x3F;
	  buf[4] = buf[4] & 0x07;
	  buf[5] = buf[5] & 0x1F;
	  buf[6] = buf[6];
	  
  	printk("<dbg>time: 20%02x-%02x-%02x  %02x:%02x:%02x\n", buf[6], buf[5], buf[3], buf[2], buf[1], buf[0]);
  }
}
#endif

}
           
static void pcf8563_get_time(struct rtc_time *rtc_tm)
{
	unsigned char buf[8];
	
	#if 0
	pcf8563_i2c_read(2, buf, 7);
	#else
	pcf8563_i2c_read(2, buf, 1);
	pcf8563_i2c_read(3, &buf[1], 1);
	pcf8563_i2c_read(4, &buf[2], 1);
	pcf8563_i2c_read(5, &buf[3], 1);
	pcf8563_i2c_read(6, &buf[4], 1);
	pcf8563_i2c_read(7, &buf[5], 1);
	pcf8563_i2c_read(8, &buf[6], 1);
	#endif

	rtc_tm->tm_sec = 	buf[0] & 0x7F;
	rtc_tm->tm_min = 	buf[1] & 0x7F;
	rtc_tm->tm_hour = buf[2] & 0x3F;
	rtc_tm->tm_mday = buf[3] & 0x3F;
	rtc_tm->tm_wday = buf[4] & 0x07;
	rtc_tm->tm_mon = 	buf[5] & 0x1F;
	rtc_tm->tm_year = buf[6];
	

#ifdef RTC_DEBUG
printk("<dbg>get_time: %X-%X-%X %X:%X:%X\n", rtc_tm->tm_year, rtc_tm->tm_mon, rtc_tm->tm_mday, rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);
#endif 

	rtc_tm->tm_sec = BCD2BIN(rtc_tm->tm_sec);
	rtc_tm->tm_min = BCD2BIN(rtc_tm->tm_min);
	rtc_tm->tm_hour = BCD2BIN(rtc_tm->tm_hour);
	rtc_tm->tm_mday = BCD2BIN(rtc_tm->tm_mday);
	rtc_tm->tm_mon = BCD2BIN(rtc_tm->tm_mon);
	rtc_tm->tm_year = BCD2BIN(rtc_tm->tm_year);
	rtc_tm->tm_wday = BCD2BIN(rtc_tm->tm_wday);
	
	
	//adjust return value
	rtc_tm->tm_year += 100;
	rtc_tm->tm_mon --;
}

static void pcf8563_set_time(struct rtc_time *rtc_tm)
{
	unsigned char buf[8];
		
	buf[0] = 	rtc_tm->tm_sec  ;
	buf[1] = 	rtc_tm->tm_min  ;
	buf[2] = 	rtc_tm->tm_hour ;
  buf[3] = 	rtc_tm->tm_mday ;
  buf[4] = 	rtc_tm->tm_wday ;
  buf[5] = 	rtc_tm->tm_mon  ;
  buf[6] = 	rtc_tm->tm_year ;
  
  pcf8563_i2c_write(2, buf, 7);
	
	#ifdef RTC_DEBUG
	printk("<dbg>set_time: %X-%X-%X %X:%X:%X\n", rtc_tm->tm_year, rtc_tm->tm_mon, rtc_tm->tm_mday, rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);
	#endif 
}

//end chendh
static rtc_ioctl(rtc_ioctl, file, cmd, arg)
{
	int ret = -EINVAL;
	struct rtc_time wtime;

//printk("\n cmd = %x,long = %x",cmd,arg);
   
              
	switch (cmd) {
		case RTC_RD_TIME:	/* Read the time/date from RTC	*/
		{
           
             // printk("\nRTC_RD_TIME");
             
			pcf8563_get_time(&wtime);
			
			ret = copy_to_user((void *)arg, &wtime, sizeof(wtime));
			if(ret)
			{
				ret = -EFAULT;
			}
			break;
		}
		case RTC_SET_TIME:	/* Set the RTC */
		{
			 //printk("\RTC_SET_TIME");
			struct rtc_time rtc_tm;
			struct rtc_time rtc_tm_set;
			unsigned char mon, day, hrs, min, sec, leap_yr;
			unsigned int yrs;
			
			if (!capable(CAP_SYS_TIME))
				return -EACCES;
	
			if (copy_from_user(&rtc_tm, (struct rtc_time*)arg,
					   sizeof(struct rtc_time)))
				return -EFAULT;
	
			yrs = rtc_tm.tm_year + 1900;
			mon = rtc_tm.tm_mon + 1;   /* tm_mon starts at zero */
			day = rtc_tm.tm_mday;
			hrs = rtc_tm.tm_hour;
			min = rtc_tm.tm_min;
			sec = rtc_tm.tm_sec;
	
			if (yrs < 2000 || yrs > 2099)
			{
				printk("Only support years between 2000 to 2099.\n");
				return -EINVAL;
			}



			leap_yr = ((!(yrs % 4) && (yrs % 100)) || !(yrs % 400));
	
			if ((mon > 12) || (day == 0))
				return -EINVAL;
	
			if (day > (days_in_mo[mon] + ((mon == 2) && leap_yr)))
				return -EINVAL;
	
			if ((hrs >= 24) || (min >= 60) || (sec >= 60))
				return -EINVAL;
	
			yrs -= 2000;
	
			sec = BIN2BCD(sec);
			min = BIN2BCD(min);
			hrs = BIN2BCD(hrs);
			day = BIN2BCD(day);
			mon = BIN2BCD(mon);
			yrs = BIN2BCD(yrs);

			rtc_tm_set.tm_year = yrs;
			rtc_tm_set.tm_mon = mon;
			rtc_tm_set.tm_mday = day;
			rtc_tm_set.tm_hour = hrs;
			rtc_tm_set.tm_min = min;
			rtc_tm_set.tm_sec = sec;


			
			pcf8563_set_time(&rtc_tm_set);
			
			return 0;
		}
		default:
			return -EINVAL;
	}
	
	return ret;
}

/*
 *	We enforce only one user at a time here with the open/close.
 *	Also clear the previous interrupt data on an open, and clean
 *	up things on a close.
 */
static int rtc_open(struct inode *inode, struct file *file)
{
	if (rtc_status & RTC_IS_OPEN) {
		return -EBUSY;
	}

	rtc_status |= RTC_IS_OPEN;

	return 0;
}

static int rtc_release(struct inode *inode, struct file *file)
{
	/*
	 * Turn off all interrupts once the device is no longer
	 * in use, and clear the data.
	 */
	rtc_status &= ~RTC_IS_OPEN;

	return 0;
}

/*
 *	The various file operations we support.
 */
static struct file_operations rtc_fops =
{
	.owner		= THIS_MODULE,
	.llseek 	= no_llseek,
	.ioctl		= rtc_ioctl,
	.open		= rtc_open,
	.release	= rtc_release,
};

static struct miscdevice rtc_dev =
{
	136,
	"rtc",
	&rtc_fops
};



//DECLARE_KCOM_GPIO_I2C();
static int __init rtc_init(void)
{
	if (misc_register(&rtc_dev)) {
		printk(KERN_ERR "rtc: cannot register misc device.\n");
		return -ENODEV;
	}
	if (!create_proc_read_entry("driver/rtc", 0, NULL, rtc_read_proc, NULL)) {
		printk(KERN_ERR "rtc: cannot create /proc/rtc.\n");
		misc_deregister(&rtc_dev);
		return -ENOENT;
	}

	rtc_freq = 1024;

	pcf8563_init();

	return 0;
}

static void __exit rtc_exit(void)
{
	/* interrupts and timer disabled at this point by rtc_release */

	remove_proc_entry ("rtc", NULL);
	misc_deregister(&rtc_dev);
}

late_initcall(rtc_init);
module_exit(rtc_exit);

/*
 *	Info exported via "/proc/rtc".
 */
static int rtc_get_status(char *buf)
{
	char *p;
	struct rtc_time tm;

	/*
	 * Just emulate the standard /proc/rtc
	 */
	p = buf;

	pcf8563_get_time(&tm);

	/*
	 * There is no way to tell if the luser has the RTC set for local
	 * time or for Universal Standard Time (GMT). Probably local though.
	 */
	p += sprintf(p,
		     "rtc_time\t: %02d:%02d:%02d\n"
		     "rtc_date\t: %04d-%02d-%02d\n"
	 	     "rtc_epoch\t: %04lu\n"
		     "24hr\t\t: yes\n",
		     tm.tm_hour, tm.tm_min, tm.tm_sec,
		     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, epoch);

	return (p - buf);
}

static int rtc_read_proc(char *page, char **start, off_t off,
                                 int count, int *eof, void *data)
{
	int len = rtc_get_status(page);
	if (len <= off+count)
		*eof = 1;
	else
		*eof = 0;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	
	return len;
}
MODULE_LICENSE("GPL");

#endif
