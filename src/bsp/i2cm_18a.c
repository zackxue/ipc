
#include "bsp/i2cm.h"

#ifdef __KERNEL__

#define I2CM_TRACE(fmt...) \
    do{printk("\033[1;31mI2C->[%s]:%d ", __FUNCTION__, __LINE__);printk(fmt);printk("\033[m\r\n");}while(0)

static spinlock_t  i2cm_mutex;
#define I2CM_LOCK() do{spin_lock(&i2cm_mutex);}while(0)
#define I2CM_UNLOCK() do{spin_unlock(&i2cm_mutex]);}while(0)


/* GPIO2_1 */
#define SCL             (1 << 1)
/* GPIO2_0 */
#define SDA             (1 << 0)
#define GPIO_2_BASE 0x20160000

#define GPIO_2_DIR IO_ADDRESS(GPIO_2_BASE + 0x400)

#define GPIO_I2C_SDA_REG IO_ADDRESS(GPIO_2_BASE + 0x04)
#define GPIO_I2C_SCL_REG IO_ADDRESS(GPIO_2_BASE + 0x08)
#define GPIO_I2C_SCLSDA_REG IO_ADDRESS(GPIO_2_BASE + 0x0c)

#define HW_REG(reg) *((volatile unsigned int *)(reg))
#define DELAY(us)       time_delay_us(us)

#define MSB_8BIT(dat) (((dat)>>8) & 0xff)
#define LSB_8BIT(dat) ((dat) & 0xff)

/* 
 * I2C by GPIO simulated  clear 0 routine.
 *
 * @param whichline: GPIO control line
 *
 */
static void i2c_clr(unsigned char whichline)
{
	unsigned char regvalue;
	
	if(whichline == SCL)
	{
		regvalue = HW_REG(GPIO_2_DIR);
		regvalue |= SCL;
		HW_REG(GPIO_2_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCL_REG) = 0;
		return;
	}
	else if(whichline == SDA)
	{
		regvalue = HW_REG(GPIO_2_DIR);
		regvalue |= SDA;
		HW_REG(GPIO_2_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SDA_REG) = 0;
		return;
	}
	else if(whichline == (SDA|SCL))
	{
		regvalue = HW_REG(GPIO_2_DIR);
		regvalue |= (SDA|SCL);
		HW_REG(GPIO_2_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCLSDA_REG) = 0;
		return;
	}
	else
	{
		printk("Error input.\n");
		return;
	}
	
}

/* 
 * I2C by GPIO simulated  set 1 routine.
 *
 * @param whichline: GPIO control line
 *
 */
static void  i2c_set(unsigned char whichline)
{
	unsigned char regvalue;
	
	if(whichline == SCL)
	{
		regvalue = HW_REG(GPIO_2_DIR);
		regvalue |= SCL;
		HW_REG(GPIO_2_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCL_REG) = SCL;
		return;
	}
	else if(whichline == SDA)
	{
		regvalue = HW_REG(GPIO_2_DIR);
		regvalue |= SDA;
		HW_REG(GPIO_2_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SDA_REG) = SDA;
		return;
	}
	else if(whichline == (SDA|SCL))
	{
		regvalue = HW_REG(GPIO_2_DIR);
		regvalue |= (SDA|SCL);
		HW_REG(GPIO_2_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCLSDA_REG) = (SDA|SCL);
		return;
	}
	else
	{
		printk("Error input.\n");
		return;
	}
}

/*
 *  delays for a specified number of micro seconds rountine.
 *
 *  @param usec: number of micro seconds to pause for
 *
 */
void time_delay_us(unsigned int usec)
{
	int i,j;
	
	for(i=0;i<usec * 5;i++)
	{
		for(j=0;j<47;j++)
		{;}
	}
}

/* 
 * I2C by GPIO simulated  read data routine.
 *
 * @return value: a bit for read 
 *
 */
 
static unsigned char i2c_data_read(void)
{
	unsigned char regvalue;
	
	regvalue = HW_REG(GPIO_2_DIR);
	regvalue &= (~SDA);
	HW_REG(GPIO_2_DIR) = regvalue;
	DELAY(1);
		
	regvalue = HW_REG(GPIO_I2C_SDA_REG);
	if((regvalue&SDA) != 0)
		return 1;
	else
		return 0;
}



/*
 * sends a start bit via I2C rountine.
 *
 */
static void i2c_start_bit(void)
{
        DELAY(1);
        i2c_set(SDA | SCL);
        DELAY(1);
        i2c_clr(SDA);
        DELAY(2);
}

/*
 * sends a stop bit via I2C rountine.
 *
 */
static void i2c_stop_bit(void)
{
        /* clock the ack */
        DELAY(1);
        i2c_set(SCL);
        DELAY(1); 
        i2c_clr(SCL);  

        /* actual stop bit */
        DELAY(1);
        i2c_clr(SDA);
        DELAY(1);
        i2c_set(SCL);
        DELAY(1);
        i2c_set(SDA);
        DELAY(1);
}

/*
 * sends a character over I2C rountine.
 *
 * @param  c: character to send
 *
 */
static void i2c_send_byte(unsigned char c)
{
    int i;
    local_irq_disable();
    for (i=0; i<8; i++)
    {
        DELAY(1);
        i2c_clr(SCL);
        DELAY(1);

        if (c & (1<<(7-i)))
            i2c_set(SDA);
        else
            i2c_clr(SDA);

        DELAY(1);
        i2c_set(SCL);
        DELAY(1);
        i2c_clr(SCL);
    }
    DELAY(1);
   // i2c_set(SDA);
    local_irq_enable();
}

/*  receives a character from I2C rountine.
 *
 *  @return value: character received
 *
 */
static unsigned char i2c_receive_byte(void)
{
    int j=0;
    int i;
    unsigned char regvalue;

    local_irq_disable();
    for (i=0; i<8; i++)
    {
        DELAY(1);
        i2c_clr(SCL);
        DELAY(2);
        i2c_set(SCL);
        
        regvalue = HW_REG(GPIO_2_DIR);
        regvalue &= (~SDA);
        HW_REG(GPIO_2_DIR) = regvalue;
        DELAY(1);
        
        if (i2c_data_read())
            j+=(1<<(7-i));

        DELAY(1);
        i2c_clr(SCL);
    }
    local_irq_enable();
    DELAY(1);
   // i2c_clr(SDA);
   // DELAY(1);

    return j;
}

/*  receives an acknowledge from I2C rountine.
 *
 *  @return value: 0--Ack received; 1--Nack received
 *          
 */
static int i2c_receive_ack(void)
{
    int nack;
    unsigned char regvalue;
    
    DELAY(1);
    
    regvalue = HW_REG(GPIO_2_DIR);
    regvalue &= (~SDA);
    HW_REG(GPIO_2_DIR) = regvalue;
    
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_set(SCL);
    DELAY(1);
    
    

    nack = i2c_data_read();

    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
  //  i2c_set(SDA);
  //  DELAY(1);

    if (nack == 0)
        return 1; 

    return 0;
}

/* 
 * sends an acknowledge over I2C rountine.
 *
 */
static void i2c_send_ack(void)
{
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_set(SDA);
    DELAY(1);
    i2c_set(SCL);
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_clr(SDA);
    DELAY(1);
}

static void i2c_send_nack(void)
{
    DELAY(1);
    i2c_set(SDA);
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    //i2c_set(SDA); //i2c_clr(SDA);
    //DELAY(1);
    i2c_set(SCL);
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_clr(SDA);
    DELAY(1);
}
 
int i2cm_write_bus8(I2CM_U8_t dev, I2CM_U8_t addr, I2CM_U8_t data)
{
	i2c_start_bit();
	i2c_send_byte((unsigned char)(dev));
	i2c_receive_ack();
	i2c_send_byte(addr);
	i2c_receive_ack();
	i2c_send_byte(data); 
	// i2c_receive_ack();//add by hyping for tw2815
	i2c_stop_bit();
	return 0;
}
EXPORT_SYMBOL(i2cm_write_bus8);

I2CM_U8_t i2cm_read_bus8(I2CM_U8_t dev, I2CM_U8_t addr)
{
	int rxdata;

	i2c_start_bit();
	i2c_send_byte((unsigned char)(dev));
	i2c_receive_ack();
	i2c_send_byte(addr);
	i2c_receive_ack();   
	i2c_start_bit();
	i2c_send_byte((unsigned char)(dev) | 1);
	i2c_receive_ack();
	rxdata = i2c_receive_byte();
	i2c_send_ack();
	i2c_stop_bit();

	return rxdata;
}
EXPORT_SYMBOL(i2cm_read_bus8);

int i2cm_write_bus16(I2CM_U8_t dev, I2CM_U16_t addr,unsigned char val)
{
	i2c_start_bit();
	i2c_send_byte(dev);
	i2c_receive_ack();
	i2c_send_byte(MSB_8BIT(addr));
	i2c_receive_ack();
	i2c_send_byte(LSB_8BIT(addr));
	i2c_receive_ack();
	i2c_send_byte(val);
	i2c_receive_ack();
	i2c_stop_bit();
	return 0;
}

I2CM_U8_t i2cm_read_bus16(I2CM_U8_t dev, I2CM_U16_t addr)
{
	int rx = 0;
	i2c_start_bit();
	i2c_send_byte(dev);
	i2c_receive_ack();
	i2c_send_byte(MSB_8BIT(addr));
	i2c_receive_ack();
	i2c_send_byte(LSB_8BIT(addr));
	i2c_receive_ack();
	i2c_start_bit();
	i2c_send_byte(dev|0x1);
	i2c_receive_ack();
	rx = i2c_receive_byte();
	i2c_stop_bit();
	return rx;
}

I2CM_U8_t i2cm_read_sccb(unsigned char devaddress, unsigned char address)
{
	int rx;
	i2c_start_bit();
	i2c_send_byte((unsigned char)(devaddress));
	i2c_receive_ack();
	i2c_send_byte(address);
	i2c_receive_ack();
	i2c_stop_bit();
	i2c_start_bit();
	i2c_send_byte((unsigned char)(devaddress) | 1);
	i2c_receive_ack();
	rx = i2c_receive_byte();
	i2c_send_ack();
	i2c_stop_bit();
	return rx;
}

int i2cm_write_sccb(unsigned char devaddress, unsigned char address, unsigned char data)
{
	i2c_start_bit();
	i2c_send_byte((unsigned char)(devaddress));
	i2c_receive_ack();
	i2c_send_byte(address);
	i2c_receive_ack();
	i2c_send_byte(data); 
	i2c_receive_ack();
	i2c_stop_bit();
	return 0;
}

unsigned short i2cm_read_ex(unsigned char devaddress, unsigned short address, unsigned int addr_len, unsigned int data_len)
{
    int i = 0;
    unsigned short tmpH = 0, tmpL = 0;
    unsigned short rxdata;
	int ret = 0;

    i2c_start_bit();
    i2c_send_byte((unsigned char)(devaddress));
    ret = i2c_receive_ack();
	printk("ret = %d\r\n", ret);
    for(i = 0; i < addr_len; i++)
    {
	    printk("gpio_i2c_read_ex: 0x%x \n\n", (unsigned char)(address >> ((addr_len - i -1) * 8)));
        i2c_send_byte((unsigned char)(address >> ((addr_len - i -1) * 8)));
        ret = i2c_receive_ack();
		printk("ret = %d\r\n", ret);
    }

	//DELAY(1);
    i2c_start_bit();
    i2c_send_byte((unsigned char)(devaddress) | 1);
    i2c_receive_ack();

    if(2 == data_len)
    {
        tmpH = i2c_receive_byte();
        i2c_send_ack();
    }

	tmpL = i2c_receive_byte();

    i2c_send_nack();
    i2c_stop_bit ();
	printk("%02x%02x\r\n", tmpH, tmpL);
    rxdata = ((tmpH << 8) & 0xff00) | (tmpL & 0xff);

    return rxdata;
}

void i2cm_write_ex(unsigned char devaddress,
                    unsigned short address, unsigned int addr_len,
                    unsigned short data, unsigned int data_len)
{
    int i = 0;

    i2c_start_bit();
    i2c_send_byte((unsigned char)(devaddress));
    i2c_receive_ack();

    for(i = 0; i < addr_len; i++)
    {
        //printk("gpio_i2c_write_ex: 0x%x.\n", (unsigned char)(address >> ((addr_len - i -1) * 8)));
        i2c_send_byte((unsigned char)(address >> ((addr_len - i -1) * 8)));
        i2c_receive_ack();
    }

    if(2 == data_len)
    {
    	i2c_send_byte((unsigned char)((data&0xff00)>>8));
    	i2c_receive_ack();
    }
	i2c_send_byte((unsigned char)(data&0x00ff));
   // i2c_receive_ack();//add by hyping for tw2815
    i2c_stop_bit();
}

static DEFINE_IOCTL(i2cm_ioctl, file, cmd, arg)
//static int i2cm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
//static int i2cm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	void __user *argp = (unsigned int __user *)arg;
	I2CPayload i2c_pt;
	memset(&i2c_pt, 0, sizeof(i2c_pt));
	if(copy_from_user(&i2c_pt, argp, sizeof(i2c_pt))){
		return -EFAULT;
	}

	switch(cmd)
	{
	case I2CM_DRV_READ:
		if(I2C_PT_BUS8 == i2c_pt.method){
			i2c_pt.bus8.val = i2cm_read_bus8(i2c_pt.bus8.dev, i2c_pt.bus8.addr);
		}else if(I2C_PT_BUS16 == i2c_pt.method){
			i2c_pt.bus16.val = i2cm_read_bus16(i2c_pt.bus16.dev, i2c_pt.bus16.addr);
		}else if(I2C_PT_SCCB == i2c_pt.method){
			i2c_pt.bus8.val = i2cm_read_sccb(i2c_pt.sccb.dev, i2c_pt.sccb.addr);
		}else if(I2C_PT_DATA16 == i2c_pt.method){
			printk("read:%02x-%04x\r\n",i2c_pt.data16.dev,  i2c_pt.data16.addr);
			i2c_pt.data16.val = i2cm_read_ex(i2c_pt.data16.dev, i2c_pt.data16.addr, 2, 2);
		}else{
			return -EINVAL;
		}
		break;
		
	case I2CM_DRV_WRITE:
		if(I2C_PT_BUS8 == i2c_pt.method){
			i2cm_write_bus8(i2c_pt.bus8.dev, i2c_pt.bus8.addr, i2c_pt.bus8.val);
		}else if(I2C_PT_BUS16 == i2c_pt.method){
			i2cm_write_bus16(i2c_pt.bus16.dev, i2c_pt.bus16.addr, i2c_pt.bus16.val);
		}else if(I2C_PT_SCCB == i2c_pt.method){
			i2cm_write_sccb(i2c_pt.sccb.dev, i2c_pt.sccb.addr, i2c_pt.sccb.val);
		}else if(I2C_PT_DATA16 == i2c_pt.method){
			printk("write:%04x-%04x\r\n", i2c_pt.data16.addr, i2c_pt.data16.val);
			i2cm_write_ex(i2c_pt.data16.dev, i2c_pt.data16.addr, 2, i2c_pt.data16.val, 2);
		}else{
			return -EINVAL;
		}
		break;
		
	default:
		return -EINVAL;
	}
	
	if(copy_to_user(argp, &i2c_pt, sizeof(i2c_pt))){
		return -EFAULT;
	}
	return 0;
}

static int i2cm_open(struct inode * inode, struct file * file)
{
	return 0;
}

static int i2cm_close(struct inode * inode, struct file * file)
{
	return 0;
}

static struct file_operations i2cm_fops = {
	.owner = THIS_MODULE,
	.ioctl = i2cm_ioctl,
	.open = i2cm_open,
	.release = i2cm_close,
};

static struct miscdevice i2cm_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "i2cm",
	.fops = &i2cm_fops,
};

int i2cm_init(void)
{
	if(misc_register(&i2cm_dev)){
		I2CM_TRACE("Register gi2c device Failed!\n");
		return -1;
	}
	I2CM_TRACE("I2C master for HI3518a @ %s %s\n", __TIME__, __DATE__);
	return 0;
}

void i2cm_exit(void)
{
	misc_deregister(&i2cm_dev);
}

module_init(i2cm_init);
module_exit(i2cm_exit);

#ifdef MODULE
//#include <linux/compile.h>
#endif
MODULE_LICENSE("LGPL");

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

#define I2CM_FILE "/dev/i2cm"

static int g_hi2cm = -1;
int I2CM_init()
{
	g_hi2cm = open(I2CM_FILE, O_RDWR);
	
	if (g_hi2cm < 0)
	{
		printf("can't open i2cm device!\n");
		return -1;
	}
	
	return 0;
}

void I2CM_destroy()
{
	if (g_hi2cm > 0)
	{
		close(g_hi2cm);
		g_hi2cm = -1;
	}
}


int I2CM_write_bus8(I2CM_U8_t dev, I2CM_U8_t addr, I2CM_U8_t val)
{
	I2CPayload i2c_pt;
	i2c_pt.method = I2C_PT_BUS8;
	i2c_pt.bus8.dev = dev;
	i2c_pt.bus8.addr = addr;
	i2c_pt.bus8.val = val;
	ioctl(g_hi2cm, I2CM_DRV_WRITE, &i2c_pt);
	return 0;
}

I2CM_U8_t I2CM_read_bus8(I2CM_U8_t dev, I2CM_U8_t addr)
{
	I2CPayload i2c_pt;
	i2c_pt.method = I2C_PT_BUS8;
	i2c_pt.bus8.dev = dev;
	i2c_pt.bus8.addr = addr;
	i2c_pt.bus8.val = 0;
	ioctl(g_hi2cm, I2CM_DRV_READ, &i2c_pt);
	return i2c_pt.bus8.val;
}

int I2CM_write_bus16(I2CM_U8_t dev, I2CM_U16_t addr, I2CM_U8_t val)
{
	I2CPayload i2c_pt;
	i2c_pt.method = I2C_PT_BUS16;
	i2c_pt.bus16.dev = dev;
	i2c_pt.bus16.addr = addr;
	i2c_pt.bus16.val = val;
	ioctl(g_hi2cm, I2CM_DRV_WRITE, &i2c_pt);
	return 0;
}

I2CM_U8_t I2CM_read_bus16(I2CM_U8_t dev, I2CM_U16_t addr)
{
	I2CPayload i2c_pt;
	i2c_pt.method = I2C_PT_BUS16;
	i2c_pt.bus16.dev = dev;
	i2c_pt.bus16.addr = addr;
	i2c_pt.bus16.val = 0;
	ioctl(g_hi2cm, I2CM_DRV_READ, &i2c_pt);
	return i2c_pt.bus16.val;
}
int I2CM_write_sccb(I2CM_U8_t dev, I2CM_U8_t addr, I2CM_U8_t val)
{
	I2CPayload i2c_pt;
	i2c_pt.method = I2C_PT_SCCB;
	i2c_pt.sccb.dev = dev;
	i2c_pt.sccb.addr = addr;
	i2c_pt.sccb.val = val;
	ioctl(g_hi2cm, I2CM_DRV_WRITE, &i2c_pt);
	return 0;
}

I2CM_U8_t I2CM_read_sccb(I2CM_U8_t dev, I2CM_U8_t addr)
{
	I2CPayload i2c_pt;
	i2c_pt.method = I2C_PT_SCCB;
	i2c_pt.sccb.dev = dev;
	i2c_pt.sccb.addr = addr;
	i2c_pt.sccb.val = 0;
	ioctl(g_hi2cm, I2CM_DRV_READ, &i2c_pt);
	return i2c_pt.bus8.val;
}

I2CM_U16_t I2CM_read_data16(I2CM_U8_t dev, I2CM_U16_t addr)
{
	I2CPayload i2c_pt;
	memset(&i2c_pt, 0, sizeof(I2CPayload));
	i2c_pt.method = I2C_PT_DATA16;
	i2c_pt.data16.dev = dev;
	i2c_pt.data16.addr = addr;
	i2c_pt.data16.val = 0;
	ioctl(g_hi2cm, I2CM_DRV_READ, &i2c_pt);
	return i2c_pt.data16.val;
}

int I2CM_write_data16(I2CM_U8_t dev, I2CM_U16_t addr, I2CM_U16_t val)
{
	I2CPayload i2c_pt;
	memset(&i2c_pt, 0, sizeof(I2CPayload));
	i2c_pt.method = I2C_PT_DATA16;
	i2c_pt.data16.dev = dev;
	i2c_pt.data16.addr = addr;
	i2c_pt.data16.val = val;
	ioctl(g_hi2cm, I2CM_DRV_WRITE, &i2c_pt);
	return 0;
}

#endif

