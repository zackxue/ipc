/* hi_rtc.c
 *
 * Copyright (c) 2012 Hisilicon Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 * History:
 *      2012.05.15 create this file <pengkang@huawei.com>
 *      2012.11.20 add temp select method  <sunny.liucan@huawei.com>
 */
 #include "bsp/rtc.h"


#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/moduleparam.h>

#include <linux/io.h>
#include <linux/irq.h>


#define RTC_NAME	"rtc_18a"
//#define HIDEBUG

#ifdef HIDEBUG
#define RTC_MSG(x...) \
do { \
	printk("%s->%d: ", __FUNCTION__, __LINE__); \
	printk(x); \
	printk("\n"); \
} while (0)	
#else
#define RTC_MSG(args...) do { } while (0)
#endif

#define RTC_ERR(x...) \
do { \
	printk(KERN_ALERT "%s->%d: ", __FUNCTION__, __LINE__); \
	printk(KERN_ALERT x); \
	printk(KERN_ALERT "\n"); \
} while (0)

#define OSDRV_MODULE_VERSION_STRING "HISI_RTC @Hi3518"

#include <asm/io.h>
#include <linux/rtc.h>

#define CRG_BASE_ADDR		IO_ADDRESS(0x20030000)
#define PERI_CRG57			(CRG_BASE_ADDR + 0xE4)
#define BIT_TEMP_SRST_REQ	2

/* RTC Control over SPI */
#define RTC_SPI_BASE_ADDR	IO_ADDRESS(0x20060000)
#define SPI_CLK_DIV			(RTC_SPI_BASE_ADDR + 0x000)
#define SPI_RW				(RTC_SPI_BASE_ADDR + 0x004)

/* RTC temperature reg */
#define RTC_TEMP_START		(RTC_SPI_BASE_ADDR + 0x80)
#define RTC_TEMP_CRC		(RTC_SPI_BASE_ADDR + 0x84)
#define RTC_TEMP_INT_MASK	(RTC_SPI_BASE_ADDR + 0x88)
#define RTC_TEMP_INT_CLEAR	(RTC_SPI_BASE_ADDR + 0x8c)
#define RTC_TEMP_STAT		(RTC_SPI_BASE_ADDR + 0x90)
#define RTC_TEMP_INT_RAW	(RTC_SPI_BASE_ADDR + 0x94)
#define RTC_TEMP_INT_STAT	(RTC_SPI_BASE_ADDR + 0x98)
#define RTC_TEMP_VAL		(RTC_SPI_BASE_ADDR + 0x9c)

#define RTC_TEMP_IRQ_NUM	(10)

/* Define the union SPI_RW */
typedef union {
	struct {
		unsigned int spi_wdata		: 8; /* [7:0] */
		unsigned int spi_rdata		: 8; /* [15:8] */
		unsigned int spi_addr		: 7; /* [22:16] */
		unsigned int spi_rw		: 1; /* [23] */
		unsigned int spi_start		: 1; /* [24] */
		unsigned int reserved		: 6; /* [30:25] */
		unsigned int spi_busy		: 1; /* [31] */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
} U_SPI_RW;

#define SPI_WRITE		(0)
#define SPI_READ		(1)

#define RTC_IRQ			(2)	

/* RTC REG */
#define RTC_10MS_COUN	0x00
#define RTC_S_COUNT  	0x01
#define RTC_M_COUNT  	0x02  
#define RTC_H_COUNT  	0x03
#define RTC_D_COUNT_L	0x04
#define RTC_D_COUNT_H	0x05

#define RTC_MR_10MS		0x06
#define RTC_MR_S		0x07
#define RTC_MR_M		0x08
#define RTC_MR_H		0x09
#define RTC_MR_D_L		0x0A
#define RTC_MR_D_H		0x0B

#define RTC_LR_10MS		0x0C
#define RTC_LR_S		0x0D
#define RTC_LR_M		0x0E
#define RTC_LR_H		0x0F
#define RTC_LR_D_L		0x10
#define RTC_LR_D_H		0x11

#define RTC_LORD		0x12

#define RTC_IMSC		0x13
#define RTC_INT_CLR		0x14
#define RTC_INT_MASK	0x15
#define RTC_INT_RAW		0x16

#define RTC_CLK			0x17
#define RTC_POR_N		0x18
#define RTC_SAR_CTRL	0x1A
#define TOT_OFFSET_L	0x1C
#define TOT_OFFSET_H	0x1D
#define TEMP_OFFSET		0x1E
#define OUTSIDE_TEMP	0x1F
#define INTERNAL_TEMP	0x20
#define TEMP_SEL		0x21
#define LUT1			0x22
#define RTC_REG_MASK	0x50

#define NORMAL_TEMP_VALUE 25
#define TEMP_TO_RTC(value) (((value) + 40)*255/180)

#define RETRY_CNT 100

const char temp_lut_table[50] = {
	0x49, 0x0,  0x5,  0xB5, 0xBC, 0xC2, 0xC8, 0xCE,
	0xD4, 0xDA, 0xDF, 0xE5, 0xEA, 0xEF, 0xF4, 0xF9,
	0xFE, 0x3,  0x7,  0xB,  0xF,  0x14, 0x17, 0x1B,
	0x1F, 0x22, 0x26, 0x29, 0x2C, 0x2F, 0x31, 0x34,
	0x37, 0x39, 0x3B, 0x3D, 0x3F, 0x41, 0x42, 0x44,
	0x45, 0x46, 0x48, 0x48, 0x49, 0x4A, 0x4A, 0x4B,
	0x4B, 0x4B
};


static DEFINE_MUTEX(hirtc_lock);
static struct timer_list temperature_timer;
static enum temp_sel_mode mode = TEMP_SEL_FIXMODE;
static int t_second = 5;
module_param(t_second, int, 0);


static int spi_rtc_write(char reg, char val)
{
	U_SPI_RW w_data, r_data;

	RTC_MSG("WRITE, reg 0x%02x, val 0x%02x", reg, val);

	r_data.u32 = 0;
	w_data.u32 = 0;
	
	w_data.bits.spi_wdata = val;
	w_data.bits.spi_addr = reg;
	w_data.bits.spi_rw = SPI_WRITE;
	w_data.bits.spi_start = 0x1;
	
	RTC_MSG("val 0x%x\n", w_data.u32);

	mutex_lock(&hirtc_lock);

	writel(w_data.u32, SPI_RW);

	do {
		r_data.u32 = readl(SPI_RW);
		RTC_MSG("read 0x%x", r_data.u32);
		RTC_MSG("test busy %d",r_data.bits.spi_busy);
	} while (r_data.bits.spi_busy);

	mutex_unlock(&hirtc_lock);
	return 0;
}

static int spi_rtc_read(char reg, char *val)
{
	U_SPI_RW w_data, r_data;

	RTC_MSG("READ, reg 0x%02x", reg);

	r_data.u32 = 0;
	w_data.u32 = 0;
	w_data.bits.spi_addr = reg;
	w_data.bits.spi_rw = SPI_READ;
	w_data.bits.spi_start = 0x1;
	
	RTC_MSG("val 0x%x\n", w_data.u32);

	mutex_lock(&hirtc_lock);

	writel(w_data.u32, SPI_RW);

	do {
		r_data.u32 = readl(SPI_RW);
		RTC_MSG("read 0x%x\n", r_data.u32);
		RTC_MSG("test busy %d\n",r_data.bits.spi_busy);
	} while (r_data.bits.spi_busy);

	mutex_unlock(&hirtc_lock);

	*val = r_data.bits.spi_rdata;
	return 0;
}

static int temp_crg_reset(void)
{
	u32 value = readl(PERI_CRG57);
	mb();
	writel(value | (1<<BIT_TEMP_SRST_REQ),  PERI_CRG57); 
	mb();
	writel(value & ~(1<<BIT_TEMP_SRST_REQ), PERI_CRG57); 

	return 0;
}

static int rtc_reset_first(void)
{
	spi_rtc_write(RTC_POR_N, 0);
	spi_rtc_write(RTC_CLK, 0x01);
	return 0;
}

/*
 * converse the data type from year.mouth.data.hour.minite.second to second
 * define 2000.1.1.0.0.0 as jumping-off point
 */
static int rtcdate2second(rtc_time_t compositetime,
		unsigned long *ptimeOfsecond)
{
	struct rtc_time tmp;

	if (compositetime.weekday > 6)
		return -1;
	
	tmp.tm_year = compositetime.year - 1900;
	tmp.tm_mon = compositetime.month - 1;
	tmp.tm_mday = compositetime.date;
	tmp.tm_wday = compositetime.weekday;
	tmp.tm_hour = compositetime.hour;
	tmp.tm_min = compositetime.minute;
	tmp.tm_sec = compositetime.second;

	if(rtc_valid_tm(&tmp))
		return -1;
		
	rtc_tm_to_time(&tmp, ptimeOfsecond);
	return 0;
}

/*
 * converse the data type from second to year.mouth.data.hour.minite.second
 * define 2000.1.1.0.0.0 as jumping-off point
 */
static int rtcSecond2Date(rtc_time_t *compositetime,
	unsigned long timeOfsecond)
{
	struct rtc_time tmp ;

	rtc_time_to_tm(timeOfsecond, &tmp);

	compositetime->year    = (unsigned int)tmp.tm_year + 1900;
	compositetime->month   = (unsigned int)tmp.tm_mon + 1;
	compositetime->date    = (unsigned int)tmp.tm_mday;
	compositetime->hour    = (unsigned int)tmp.tm_hour;
	compositetime->minute  = (unsigned int)tmp.tm_min;
	compositetime->second  = (unsigned int)tmp.tm_sec;
	compositetime->weekday = (unsigned int)tmp.tm_wday;

	RTC_MSG("RTC read time");
	RTC_MSG("\tyear %d", compositetime->year);
	RTC_MSG("\tmonth %d", compositetime->month);
	RTC_MSG("\tdate %d", compositetime->date);
	RTC_MSG("\thour %d", compositetime->hour);
	RTC_MSG("\tminute %d", compositetime->minute);
	RTC_MSG("\tsecond %d", compositetime->second);
	RTC_MSG("\tweekday %d", compositetime->weekday);

	return 0;
}

static int hirtc_get_alarm(rtc_time_t *compositetime)
{
	unsigned char dayl, dayh;
	unsigned char second, minute, hour;
	unsigned long seconds = 0;
	unsigned int day;

	spi_rtc_read(RTC_MR_S, &second);
	spi_rtc_read(RTC_MR_M, &minute);
	spi_rtc_read(RTC_MR_H, &hour);
	spi_rtc_read(RTC_MR_D_L, &dayl);
	spi_rtc_read(RTC_MR_D_H, &dayh);
	day = (unsigned int)(dayl | (dayh << 8)); 
	RTC_MSG("day %d\n", day);
	seconds = second + minute*60 + hour*60*60 + day*24*60*60;
	
	rtcSecond2Date(compositetime, seconds);
	return 0;
}

static int hirtc_set_alarm(rtc_time_t compositetime)
{
	unsigned int days;
	unsigned long seconds = 0;

	RTC_MSG("RTC read time");
	RTC_MSG("\tyear %d", compositetime.year);
	RTC_MSG("\tmonth %d", compositetime.month);
	RTC_MSG("\tdate %d", compositetime.date);
	RTC_MSG("\thour %d", compositetime.hour);
	RTC_MSG("\tminute %d", compositetime.minute);
	RTC_MSG("\tsecond %d", compositetime.second);
	RTC_MSG("\tweekday %d", compositetime.weekday);

	rtcdate2second(compositetime, &seconds);
	days = seconds/(60*60*24);

	spi_rtc_write(RTC_MR_10MS, 0);
	spi_rtc_write(RTC_MR_S, compositetime.second);
	spi_rtc_write(RTC_MR_M, compositetime.minute);
	spi_rtc_write(RTC_MR_H, compositetime.hour);
	spi_rtc_write(RTC_MR_D_L, (days & 0xFF));
	spi_rtc_write(RTC_MR_D_H, (days >> 8));
	return 0;
}

static int hirtc_get_time(rtc_time_t *compositetime)
{
	unsigned char dayl, dayh;
	unsigned char second, minute, hour;
	unsigned long seconds = 0;
	unsigned int day;

	spi_rtc_read(RTC_S_COUNT, &second);
	spi_rtc_read(RTC_M_COUNT, &minute);
	spi_rtc_read(RTC_H_COUNT, &hour);
	spi_rtc_read(RTC_D_COUNT_L, &dayl);
	spi_rtc_read(RTC_D_COUNT_H, &dayh);
	day = (dayl | (dayh << 8)); 

	RTC_MSG("day %d\n", day);

	seconds = second + minute*60 + hour*60*60 + day*24*60*60;

	rtcSecond2Date(compositetime, seconds);
	return 0;
}

static int hirtc_set_time(rtc_time_t compositetime)
{
	unsigned char ret;
	unsigned int days;
	unsigned long seconds = 0;
	unsigned int cnt = 0;

	RTC_MSG("RTC read time");
	RTC_MSG("\tyear %d", compositetime.year);
	RTC_MSG("\tmonth %d", compositetime.month);
	RTC_MSG("\tdate %d", compositetime.date);
	RTC_MSG("\thour %d", compositetime.hour);
	RTC_MSG("\tminute %d", compositetime.minute);
	RTC_MSG("\tsecond %d", compositetime.second);
	RTC_MSG("\tweekday %d", compositetime.weekday);

	rtcdate2second(compositetime, &seconds);
	days = seconds/(60*60*24);

	RTC_MSG("day %d\n", days);

	do {
		spi_rtc_read(RTC_LORD, &ret);
		msleep(1);
		cnt++;
	} while (ret && (cnt < RETRY_CNT));
	
	if (cnt >= RETRY_CNT) {
		RTC_ERR("check state error!\n");
		return -1;
	}	

	spi_rtc_write(RTC_LR_10MS, 0);
	spi_rtc_write(RTC_LR_S, compositetime.second);
	spi_rtc_write(RTC_LR_M, compositetime.minute);
	spi_rtc_write(RTC_LR_H, compositetime.hour);
	spi_rtc_write(RTC_LR_D_L, (days & 0xFF));
	spi_rtc_write(RTC_LR_D_H, (days >> 8));

	spi_rtc_write(RTC_LORD, 1);

	RTC_MSG("set time ok!\n");
	return 0;
}

static long hi_rtc_ioctl(struct file *file,
		unsigned int cmd,
		unsigned long arg)
{
	unsigned long flags;
	int ret;

	rtc_time_t time;
	reg_data_t reg_info;
	reg_temp_mode_t temp_mode;

	switch (cmd)
	{
	/* Alarm interrupt on/off */
	case HI_RTC_AIE_ON:
		RTC_MSG("HI_RTC_AIE_ON");
		spi_rtc_write(RTC_IMSC, 0x1);
		break;

	case HI_RTC_AIE_OFF:
		RTC_MSG("HI_RTC_AIE_OFF");
		spi_rtc_write(RTC_IMSC, 0x0);
		break;
	
	case HI_RTC_ALM_READ:
		hirtc_get_alarm(&time);
		if (copy_to_user((void *)arg, &time, sizeof(time)))
			return -EFAULT;
		break;

	case HI_RTC_ALM_SET:
		if (copy_from_user(&time, (struct rtc_time_t*) arg,
			sizeof(time)))
			return -EFAULT;
		ret = hirtc_set_alarm(time);
		if (ret){
			return -EFAULT;
		}
		break;
	
	case HI_RTC_RD_TIME:
		RTC_MSG("HI_RTC_RD_TIME");
		hirtc_get_time(&time);
		ret = copy_to_user((void *)arg, &time, sizeof(time));
		if (ret) {
			return -EFAULT;
		}
		break;

	case HI_RTC_SET_TIME:
		RTC_MSG("HI_RTC_SET_TIME");
		ret = copy_from_user(&time,
				(struct rtc_time_t *) arg,
				sizeof(time));
		if (ret) {
			return -EFAULT;
		}
		hirtc_set_time(time);
		break;

	case HI_RTC_RESET:
		rtc_reset_first();
		break;
	
	case HI_RTC_REG_SET:
		ret = copy_from_user(&reg_info, (struct reg_data_t *) arg,
			sizeof(reg_info));
		if (ret) {
			return -EFAULT;
		}
		spi_rtc_write(reg_info.reg, reg_info.val);
		break;

	case HI_RTC_REG_READ:
		ret = copy_from_user(&reg_info, (struct reg_data_t *) arg,
			sizeof(reg_info));
		if (ret) {
			return -EFAULT;
		}

		spi_rtc_read(reg_info.reg, &reg_info.val);
		ret = copy_to_user((void *)arg, &reg_info, sizeof(reg_info));
		if (ret) {
			return -EFAULT;
		}
		break;

	case HI_RTC_SET_TEMP_MODE:
		ret = copy_from_user(&temp_mode, (struct reg_temp_mode_t *) arg,
				sizeof(temp_mode));
		if (ret) {
			return -EFAULT;
		}
		
		local_irq_save(flags);
		mode = temp_mode.mode;
		local_irq_restore(flags);
		
		switch (mode) {
		case TEMP_SEL_FIXMODE:
			if (temp_mode.value > 140 || temp_mode.value < -40)
			{
				RTC_MSG("invalid value %d", temp_mode.value);
				return -EFAULT;
			}

			if (timer_pending(&temperature_timer))
				del_timer(&temperature_timer);
			
			spi_rtc_write(OUTSIDE_TEMP, TEMP_TO_RTC(temp_mode.value));
			break;

		case TEMP_SEL_INTERNAL:
		case TEMP_SEL_OUTSIDE:
			mod_timer(&temperature_timer, jiffies + HZ);
			break;

		default:
			return -EFAULT;
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}



static void temperature_detection(unsigned long arg)
{
	int ret;
	int cnt = 0;
	
	RTC_MSG("START temperature adjust");

	if (mode == TEMP_SEL_OUTSIDE)
	{	

		do {
			ret = readl(RTC_TEMP_STAT);
			udelay(10);
			cnt++;
		} while (ret && (cnt < RETRY_CNT));

		if (cnt >= RETRY_CNT)
		{
			/*maybe temperature capture module is error,
			  need reset */
			RTC_ERR("RTC_TEMP_STAT not ready,please check pin configuration\n");		
			temp_crg_reset();
			goto timer_again;
		}	
		
		RTC_MSG("WRITE RTC_TEMP_START");

		writel(0x1, RTC_TEMP_START);	
	}
	else if (mode == TEMP_SEL_INTERNAL)
	{
		char temp = TEMP_TO_RTC(25);
	   	spi_rtc_read(INTERNAL_TEMP, &temp);

		RTC_MSG("internal temp is %d\n", temp);

		/*FIXME: sub offset to get enviroment temperature*/
	    	
		spi_rtc_write(OUTSIDE_TEMP, temp);
	}
	else {
		RTC_ERR("invalid temperature mode");
	}

timer_again:
	mod_timer(&temperature_timer, jiffies + HZ*t_second);
}

/**
 * this function change DS1820 temperature format to native RTC format
 * OUTSIDE_TEMP value size (-40<oC>, +140<oC>), use 8bit to spec
 * DS1820 value size (-55<oC>, +120<oC>), use 9bit to spec
 */
static void set_temperature(void)
{
	unsigned short ret;
	unsigned char temp;

	ret = (unsigned short)readl(RTC_TEMP_VAL);
	RTC_MSG("READ DS1820 temperature value 0x%x", ret);

	/* mode 1 sample, ret > 0x800 means negative number */
	if (ret > 0x800) {
		/* change to positive number */
		ret = 4096-ret;
		ret = ret/2;
		/* rtc temperature lower limit -40<oC> */
		if (ret > 40)
			ret = 40;
		temp = (40-ret)*255/180;
	} else {
		/* rtc temperature upper limit 140<oC> */
		ret = ret/2;
		if (ret > 140)
			ret = 140;
		temp = TEMP_TO_RTC(ret);
	}

	RTC_MSG("WRITE RTC temperature value 0x%02x", temp);

	spi_rtc_write(OUTSIDE_TEMP, temp);
}

static irqreturn_t rtc_temperature_interrupt(int irq, void *dev_id)
{
	int ret = IRQ_NONE;
	int irq_stat;

	irq_stat = readl(RTC_TEMP_INT_STAT);
	if (!irq_stat) {
		return ret;
	}

	RTC_MSG("irq mask");
	writel(0x01, RTC_TEMP_INT_MASK);

	irq_stat = readl(RTC_TEMP_INT_RAW);
	RTC_MSG("irq clear");

	writel(0x1, RTC_TEMP_INT_CLEAR);

	if (mode != TEMP_SEL_OUTSIDE)
		goto endl;

	if (irq_stat & 0x2) {
		/* err start again */
		mod_timer(&temperature_timer, jiffies + HZ*t_second);
	} else {
		/* set temperature data */
		set_temperature();
	}

endl:	
	RTC_MSG("irq unmask");
	writel(0x0, RTC_TEMP_INT_MASK);
	
	ret = IRQ_HANDLED;
	return ret;
}

/*
 * interrupt function
 * do nothing. left for future
 */
static irqreturn_t rtc_alm_interrupt(int irq, void *dev_id)
{
	int ret = IRQ_NONE;	

	printk(KERN_WARNING "RTC alarm interrupt!!!\n");

	//spi_rtc_write(RTC_IMSC, 0x0);
	spi_rtc_write(RTC_INT_CLR, 0x1);
	//spi_rtc_write(RTC_IMSC, 0x1);

	/*FIXME: do what you want here. such as wake up a pending thread.*/

	ret = IRQ_HANDLED;
	return ret;
}


static int hi_rtc_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int hi_rtc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations hi_rtc_fops = {
	.owner	= THIS_MODULE,
	.unlocked_ioctl = hi_rtc_ioctl,
	.open = hi_rtc_open,
	.release = hi_rtc_release,
};

static struct miscdevice rtc_char_driver = {
	.minor	= RTC_MINOR,
	.name	= RTC_NAME,
	.fops	= &hi_rtc_fops
};

static int __init rtc_init(void)
{
	int i,ret = 0;
	char base;

	ret = misc_register(&rtc_char_driver);
	if (0 != ret)
	{
		RTC_ERR("rtc device register failed!\n");
		return -EFAULT;
	}

	ret = request_irq(RTC_IRQ, &rtc_alm_interrupt, 0,
			"RTC Alarm", NULL);
	if(0 != ret)
	{
		RTC_ERR("hi3518 rtc: failed to register IRQ(%d)\n", RTC_IRQ);
		goto RTC_INIT_FAIL1;
	}

	ret = request_irq(RTC_TEMP_IRQ_NUM,
			&rtc_temperature_interrupt,
			//IRQF_SHARED,
			0, "RTC Temperature", NULL);
	if(0 != ret)
	{
		RTC_ERR("hi3518 rtc: failed to register IRQ(%d)\n",
				RTC_TEMP_IRQ_NUM);
		goto RTC_INIT_FAIL2;
	}

	init_timer(&temperature_timer);
	temperature_timer.function = temperature_detection;
	temperature_timer.expires = jiffies + HZ*t_second;

	/* clk div value = (apb_clk/spi_clk)/2-1, for asic ,
	   apb clk = 100MHz, spi_clk = 10MHz,so value= 0x4 */
	writel(0x4, SPI_CLK_DIV);

	/* always update temperature from TEMP_OUTSIDE */
	spi_rtc_write(TEMP_SEL, 0x01);
	spi_rtc_write(RTC_SAR_CTRL, 0x08);
	spi_rtc_write(OUTSIDE_TEMP, TEMP_TO_RTC(25));

	/*init rtc temperature lut table value*/
	for (i = 0; i < sizeof(temp_lut_table)/sizeof(temp_lut_table[0]); i++)
	{	
		if (i < 3)
			base = TOT_OFFSET_L;
		else
			base = LUT1;

		spi_rtc_write(base + i, temp_lut_table[i]);	
	}

	/* enable temperature IRQ */
	writel(0x0, RTC_TEMP_INT_MASK);
	return 0;

RTC_INIT_FAIL2:
	free_irq(RTC_IRQ, NULL);
RTC_INIT_FAIL1:
	misc_deregister(&rtc_char_driver);
	return ret;
}

static void __exit rtc_exit(void)
{
	del_timer(&temperature_timer);
	free_irq(RTC_IRQ, NULL);
	free_irq(RTC_TEMP_IRQ_NUM, NULL);
	misc_deregister(&rtc_char_driver);
}

module_init(rtc_init);
module_exit(rtc_exit);

MODULE_AUTHOR("Digital Media Team ,Hisilicon crop ");
MODULE_DESCRIPTION("Real Time Clock interface for HI3518");
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);
MODULE_LICENSE("GPL");

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define RTC_FILE "/dev/rtc_18a"
//#define RTC_FILE "/dev/hi_rtc"


static int g_rtc18a = -1;

int RTC_init(unsigned char bsync)
{
	g_rtc18a = open(RTC_FILE, O_RDWR);
	
	if (g_rtc18a < 0)
	{
		printf("can't open RTC device :%s!\n", RTC_FILE);
		return -1;
	}
	if(bsync){
		RTC_sync_to_system();
	}
	return 0;
}


void RTC_destroy()
{
	if (g_rtc18a > 0)
	{
		close(g_rtc18a);
		g_rtc18a = -1;
	}
}

#if 0
struct tm {
        int tm_sec;     /* seconds after the minute - [0,61] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#endif

char *sMonth[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
#define WEEK_STR "WEEK"

int RTC_gettime(rtc_time_t *rtc_tm)
{
	if(g_rtc18a != -1){
		ioctl(g_rtc18a, HI_RTC_RD_TIME, rtc_tm);
		printf("%s %s %02d %02d:%02d:%02d %04d 0.000000 seconds\n", 
			"get", sMonth[rtc_tm->month-1], rtc_tm->date, rtc_tm->hour,
			rtc_tm->minute, rtc_tm->second, rtc_tm->year);
	}else{
		printf("rtc isn't init!\r\n");
	}

}

int RTC_settime(time_t curtime)
{
	struct tm *cur_tm;
	rtc_time_t rtc_time;
	//localtime_r(&curtime, &cur_tm);
	printf("%s-%d\r\n", __FUNCTION__, curtime);
	cur_tm = gmtime(&curtime);
	rtc_time.year = cur_tm->tm_year + 1900;
	rtc_time.month = cur_tm->tm_mon + 1;
	rtc_time.date = cur_tm->tm_mday;
	rtc_time.hour = cur_tm->tm_hour;
	rtc_time.minute = cur_tm->tm_min;
	rtc_time.second = cur_tm->tm_sec;
	if(g_rtc18a != -1){
		ioctl(g_rtc18a, HI_RTC_SET_TIME, &rtc_time);
		printf("%s %s %02d %02d:%02d:%02d %04d 0.000000 seconds\n", 
			"set", sMonth[rtc_time.month-1], rtc_time.date, rtc_time.hour,
			rtc_time.minute, rtc_time.second, rtc_time.year);
	}else{
		printf("rtc isn't init!\r\n");
	}
}

int RTC_sync_to_system()
{
	rtc_time_t curtime;
	struct tm time_tm;
	if(g_rtc18a != -1){
		ioctl(g_rtc18a, HI_RTC_RD_TIME, &curtime);
		time_tm.tm_year = curtime.year - 1900;
		time_tm.tm_mon = curtime.month - 1;
		time_tm.tm_mday = curtime.date;
		time_tm.tm_hour = curtime.hour;
		time_tm.tm_min = curtime.minute;
		time_tm.tm_sec = curtime.second;
		time_t timet = mktime(&time_tm);
		struct timeval tv;
		tv.tv_sec = timet;
		tv.tv_usec = 0;
		settimeofday(&tv,NULL);
		printf("%s->%04d/%02d/%02d %02d:%02d:%02d\r\n", __FUNCTION__,
			curtime.year, curtime.month,
			curtime.date, curtime.hour,
			curtime.minute, curtime.second);
	}else{
		printf("rtc isn't init!\r\n");
	}
}

#endif//#ifdef __KERNEL__

