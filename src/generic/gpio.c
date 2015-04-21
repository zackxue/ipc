#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpio.h"


#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct tag_MMAP_Node
{
	unsigned int Start_P;
	unsigned int Start_V;
	unsigned int length;
    unsigned int refcount;  /* map后的空间段的引用计数 */
	struct tag_MMAP_Node * next;
}MMAP_Node_t;

MMAP_Node_t * pMMAPNode = NULL;

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK 0xfffff000

static int fd = -1;
static const char dev[]="/dev/mem";
static void * gpio_addr_begin;


/* no need considering page_size of 4K */
static void * memmap(unsigned int phy_addr, unsigned int size)
{
	unsigned int phy_addr_in_page;
	unsigned int page_diff;

	unsigned int size_in_page;

	MMAP_Node_t * pTmp;
	MMAP_Node_t * pNew;
	
	void *addr=NULL;

	if(size == 0)
	{
		printf("memmap():size can't be zero!\n");
		return NULL;
	}

	/* check if the physical memory space have been mmaped */
	pTmp = pMMAPNode;
	while(pTmp != NULL)
	{
		if( (phy_addr >= pTmp->Start_P) && 
			( (phy_addr + size) <= (pTmp->Start_P + pTmp->length) ) )
		{
            pTmp->refcount++;   /* referrence count increase by 1  */
			return (void *)(pTmp->Start_V + phy_addr - pTmp->Start_P);
		}

		pTmp = pTmp->next;
	}

	/* not mmaped yet */
	if(fd < 0)
	{
		/* dev not opened yet, so open it */
		fd = open (dev, O_RDWR | O_SYNC);
		if (fd < 0)
		{
			printf("memmap():open %s error!\n", dev);
			return NULL;
		}
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page =((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

	addr = mmap ((void *)0, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
	if (addr == MAP_FAILED)
	{
		printf("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}

	/* add this mmap to MMAP Node */
	pNew = (MMAP_Node_t *)malloc(sizeof(MMAP_Node_t));
    if(NULL == pNew)
    {
        printf("memmap():malloc new node failed!\n");
        return NULL;
    }
	pNew->Start_P = phy_addr_in_page;
	pNew->Start_V = (unsigned int)addr;
	pNew->length = size_in_page;
    pNew->refcount = 1;
	pNew->next = NULL;
	
	if(pMMAPNode == NULL)
	{
		pMMAPNode = pNew;
	}
	else
	{
		pTmp = pMMAPNode;
		while(pTmp->next != NULL)
		{
			pTmp = pTmp->next;
		}

		pTmp->next = pNew;
	}

	return (void *)(addr+page_diff);
}

#define GPIO_ADDR_INIT(addr) addr = memmap(0x101e0000, 0x1b000)
#define GPIO_SCTL_ADDR gpio_addr_begin
#define GPIO_SCTL_PERCTRL1 (GPIO_SCTL_ADDR+0x020)
#define GPIO_ADDRESS(addr) (GPIO_SCTL_ADDR + (addr - 0x101e0000))

static unsigned int get_gpio_base_addr(int gpio_group)
{
	unsigned int gpio_base_addr;
	if(gpio_group >= 4){
		gpio_base_addr = (unsigned int)(GPIO_ADDRESS(0x101f3000) + gpio_group*0x1000);
	}else{
		gpio_base_addr = (unsigned int)(GPIO_ADDRESS(0x101e4000) + gpio_group*0x1000);
	}
	return gpio_base_addr;
}

static void set_gpio_in(int gpio_group, int gpio_pin)
{
	unsigned char result;
	unsigned int gpio_addr = get_gpio_base_addr(gpio_group);
	result=(*((volatile unsigned char *)(gpio_addr + 0x400)));	
	result &=~(1 << gpio_pin);		
	*((volatile unsigned char *)(gpio_addr + 0x400)) = result;	
}

static void set_gpio_out(int gpio_group, int gpio_pin)
{
	unsigned char result;	
	unsigned int gpio_addr = get_gpio_base_addr(gpio_group);
	
	result=(*((volatile unsigned char *)(gpio_addr + 0x400)));	 
	result |= (1 << gpio_pin);		
	*((volatile unsigned char *)(gpio_addr + 0x400)) = result;	
}

static unsigned char _gpio_read_bit(int gpio_group, int gpio_pin)
{
	unsigned int gpio_addr = get_gpio_base_addr(gpio_group);
	return (*((volatile unsigned char *)(gpio_addr+(0x1<<(gpio_pin+2)))) >> gpio_pin) & 1;
}

static unsigned char _gpio_write_bit(int gpio_group, int gpio_pin, unsigned char value)
{
	unsigned int gpio_addr;
	gpio_addr = get_gpio_base_addr(gpio_group);
	
	unsigned char result;        
    result=(*((volatile unsigned char *)(gpio_addr+(0x1<<(gpio_pin+2)))));    
    if(value){	
    	result|=(value<<gpio_pin);    
    }else{	
		result&=~(1<<gpio_pin);    
    }	
    *((volatile unsigned char *)(gpio_addr+(0x1<<(gpio_pin+2))))=result;    
	return 0;
}

static int ircut_gpio_init()
{
	unsigned long dwRetval;
	dwRetval=*((volatile unsigned long *)GPIO_SCTL_PERCTRL1);
	dwRetval |= (1<<3);
    *((volatile unsigned long *)GPIO_SCTL_PERCTRL1)=dwRetval;
	set_gpio_in(7, 7);//GPIO7_7

	return 0;
}

unsigned char GPIO_read_bit(const char *name)
{
	if(!strcmp(name, "ircut")){
		return _gpio_read_bit(7, 7);//GPIO7_7
	}else{
		printf("wrong GPIO type!\r\n");
		return 0;
	}
}

unsigned char GPIO_write_bit(const char *name, unsigned char value)
{
	if(!strcmp(name, "ircut")){
		return _gpio_write_bit(7, 7, value);//GPIO7_7
	}else{
		printf("wrong GPIO type!\r\n");
		return 0;
	}
}



int GPIO_init()
{
	GPIO_ADDR_INIT(gpio_addr_begin);
	//IRCUT  GPIO7_7
	ircut_gpio_init();
	return 0;
}


void GPIO_destroy()
{

}
