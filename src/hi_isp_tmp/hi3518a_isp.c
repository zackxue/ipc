


#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"
#include "sdk_debug.h"
#include "sdk/sdk_api.h"
#include "sdk_isp.h" // in hi_isp_tmp

#include "signal.h"
#include "hi_ssp.h"

#define HI3518A_VIN_DEV (0)
#define HI3518A_VIN_CHN (0)

#define GPIO_BASE_ADDR 0x20140000
//ir-cut led :GPIO0_0
#define IRCUT_LED_GPIO_PINMUX_ADDR 0x200f0120
#define IRCUT_LED_GPIO_DIR_ADDR 0x20140400
#define IRCUT_LED_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_LED_GPIO_PIN 0
#define IRCUT_LED_GPIO_GROUP 0

//new hardware ir-cut control :GPIO0_2
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0128
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 2
#define NEW_IRCUT_CTRL_GPIO_GROUP 0

//old hardware ir-cut control :GPIO0_4
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0130
#define IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_CTRL_GPIO_PIN 4
#define IRCUT_CTRL_GPIO_GROUP 0

//ir-cut photoswitch read:GPIO0_6
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 0x200f0138
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 0x20140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 6
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 0

//default factory reset:GPIO0_7
#define HW_RESET_GPIO_PINMUX_ADDR 0x200f013c
#define HW_RESET_GPIO_DIR_ADDR 0x20140400
#define HW_RESET_GPIO_DATA_ADDR 0x201403fc
#define HW_RESET_GPIO_PIN 7
#define HW_RESET_GPIO_GROUP 0

#define ISP_GPIO_DAYLIGHT (0)
#define ISP_GPIO_NIGHT (1)

static uint32_t gs_gpio_status_old = ISP_GPIO_DAYLIGHT;//daytime


static uint32_t isp_gpio_get_dir_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x400;
	return ret_val;
}

static uint32_t isp_gpio_get_data_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x3fc;
	return ret_val;
}

static uint32_t isp_gpio_pin_read(int gpio_group, int gpio_pin)
{
	uint32_t reg_val = 0;

	//pin dir :in
	sdk_sys->read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);

	//read pin
	sdk_sys->read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= (1<<gpio_pin);
	return reg_val;
}

static void isp_gpio_pin_write(int gpio_group, int gpio_pin, uint8_t val)
{
	uint32_t reg_val = 0;
	
	//pin dir :out
	sdk_sys->read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val |= (1<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);
	
	sdk_sys->read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	//printf("gpio%d_%d:before reg_val%x\r\n", gpio_group, gpio_pin, reg_val);
	reg_val &= ~(1<<gpio_pin);
	reg_val |= (val<<gpio_pin);
	//printf("gpio%d_%d:after reg_val%x\r\n", gpio_group, gpio_pin, reg_val);
	sdk_sys->write_reg(isp_gpio_get_data_addr(gpio_group), reg_val);
}

static void isp_ircut_control_daylight()
{
	printf("%s\r\n", __FUNCTION__);
	isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
	isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);	
}

static void isp_ircut_control_night()
{
	printf("%s\r\n", __FUNCTION__);
	isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);
	isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);	
}


static void isp_ircut_switch(uint8_t bEnable)//0:daytime   1:night
{
	static uint32_t old_satuation = 0;
	if(!old_satuation){
			SOC_CHECK(HI_MPI_ISP_GetSaturation(&old_satuation));
		}
	if(!bEnable){
		printf("daylight mode!\r\n");	
		SOC_CHECK(HI_MPI_ISP_SetSaturation(old_satuation));
		printf("saturation:%d\r\n", old_satuation);
		isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
		isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);	
		gs_gpio_status_old = ISP_GPIO_DAYLIGHT;
		signal(SIGALRM,isp_ircut_control_daylight); 
		alarm(2); 
	}else{			
		printf("night mode!\r\n");
		SOC_CHECK(HI_MPI_ISP_SetSaturation(0));
		isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1);//IR LED on
		isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);//IR-CUT on
		isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
		gs_gpio_status_old = ISP_GPIO_NIGHT;
		signal(SIGALRM,isp_ircut_control_night); 
		alarm(2); 
	}
}

int exposure_calculate(time_t cur_time)
{
	
	 ISP_INNER_STATE_INFO_S pstInnerStateInfo;
	 SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfo(&pstInnerStateInfo));
	 printf("u32ExposureTime = 	0x%08x\r\n", pstInnerStateInfo.u32ExposureTime);
	 printf("u32AnalogGain = 	0x%04x\r\n", pstInnerStateInfo.u32AnalogGain);
	 printf("u32DigitalGain = 	0x%04x\r\n", pstInnerStateInfo.u32DigitalGain);
	 printf("u32Exposure = 		0x%04x\r\n", pstInnerStateInfo.u32Exposure);
	 //printf("u8AveLum = 			0x%02x\r\n", pstInnerStateInfo.u8AveLum);
	 //printf("bExposureIsMAX = 	0x%x\r\n", pstInnerStateInfo.bExposureIsMAX);
	/* ISP_AE_ATTR_S  pstAEAttr; 
	 HI_MPI_ISP_GetAEAttr(&pstAEAttr); 
	 printf("enAEMode = 			0x%x\r\n", pstAEAttr.enAEMode);
	 printf("u16ExpTimeMax = 	0x%04x\r\n", pstAEAttr.u16ExpTimeMax);
	 printf("u16ExpTimeMin = 	0x%04x\r\n", pstAEAttr.u16ExpTimeMin);
	 printf("u16DGainMax = 		0x%04x\r\n", pstAEAttr.u16DGainMax);
	 printf("u16DGainMin = 		0x%04x\r\n", pstAEAttr.u16DGainMin);
	 printf("u16AGainMax = 		0x%04x\r\n", pstAEAttr.u16AGainMax);
	 printf("u16AGainMin = 		0x%04x\r\n", pstAEAttr.u16AGainMin);
	 printf("s16ExpTolerance = 		0x%04x\r\n", pstAEAttr.s16ExpTolerance);
	 printf("u8ExpStep = 		0x%04x\r\n", pstAEAttr.u8ExpStep);*/
	 return 0;
}

static uint32_t isp_get_iso()
{
	ISP_INNER_STATE_INFO_S InnerStateInfo;
	SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfo(&InnerStateInfo));
	HI_U32 _again = InnerStateInfo.u32AnalogGain == 0 ? 1 : InnerStateInfo.u32AnalogGain;
	HI_U32 _dgain = InnerStateInfo.u32DigitalGain == 0 ? 1 : InnerStateInfo.u32DigitalGain;
	return InnerStateInfo.u32AnalogGain * InnerStateInfo.u32DigitalGain * 100;
}

static uint8_t sdk_isp_calculate_exposure(uint32_t old_state)
{
	//ISP_EXP_STA_INFO_S pstExpStatistic;
	//SOC_CHECK(HI_MPI_ISP_GetExpStaInfo(&pstExpStatistic));
	uint8_t ret_val = 0;
	//HI_U32 switch_area[2] = {0x20fa, 0x2544}; 
	HI_U32 switch_area[2] = {0x2844, 0x67};
	ISP_INNER_STATE_INFO_S pstInnerStateInfo;
	SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfo(&pstInnerStateInfo));
	printf("aveLum = 0x%04x/%02x\r\n", pstInnerStateInfo.u32Exposure, pstInnerStateInfo.u8AveLum);

	if(!old_state){
		if(pstInnerStateInfo.u32Exposure > switch_area[0]){
			ret_val = 1;
		}else{
			ret_val = 0;
		}
	}else{
		if(pstInnerStateInfo.u32Exposure < switch_area[1]){
			ret_val = 0;
		}else{
			ret_val = 1;
		}
	}
	printf("old_state:%d/%d\r\n", old_state, ret_val);
	return ret_val;//0:daytime 1:night
}

static void isp_ircut_gpio_init()
{
	uint32_t reg_val = 0;
	//muxpin
	sdk_sys->write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, 0);//GPIO0_0
	//pin dir :out
	sdk_sys->read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_LED_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);

	//muxpin
	sdk_sys->write_reg(NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR, 0);//GPIO0_2
	//pin dir :out
	sdk_sys->read_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<NEW_IRCUT_CTRL_GPIO_PIN);
	sdk_sys->write_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	
	//muxpin
	sdk_sys->write_reg(IRCUT_CTRL_GPIO_PINMUX_ADDR, 0);//GPIO0_4
	//pin dir :out
	sdk_sys->read_reg(IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_CTRL_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);
	
	//muxpin
	sdk_sys->write_reg(IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR, 0);//GPIO0_6
	//pin dir :in
	sdk_sys->read_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<IRCUT_PHOTOSWITCH_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, reg_val);
	
	//muxpin
	sdk_sys->write_reg(HW_RESET_GPIO_PINMUX_ADDR, 1);//GPIO0_7
	//pin dir :in
	sdk_sys->read_reg(HW_RESET_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<HW_RESET_GPIO_PIN);
	sdk_sys->write_reg(HW_RESET_GPIO_DIR_ADDR, reg_val);

	isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
	isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
	
}


#include "hi_i2c.h"

static int ar0130_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x20;		/* I2C Address of AR0130 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

    i2c_data.dev_addr = sensor_i2c_addr ;
    i2c_data.reg_addr = addr    ;
    i2c_data.addr_byte_num   = sensor_addr_byte  ;
    i2c_data.data_byte_num   = sensor_data_byte ;
    ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
    *ret_data =  i2c_data.data ;
    printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);
	return 0;
}

static int ar0130_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x20;		/* I2C Address of AR0130 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int ov9712_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of ov9712 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int ov9712_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of ov9712 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int soih22_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of soih22 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int soih22_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of soih22 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int imx122_i2c_read(int addr, uint16_t* ret_data)
{
	unsigned int data = (unsigned int)(((addr&0xffff)<<8));
	int fd = -1;
	int ret;
	unsigned int value;

	fd = open("/dev/ssp", 0);
	if(fd < 0)
	{
		printf("Open /dev/ssp error!\n");
		return -1;
	}

	value = data;

	ret = ioctl(fd, SSP_READ_ALT, &value);

	close(fd);
	*ret_data = value&0xff;
	return (value&0xff);
}

static int imx122_i2c_write(int addr, int data)
{
	unsigned int value = (unsigned int)(((addr&0xffff)<<8) | (data & 0xff));

	int fd = -1;
	int ret;

	fd = open("/dev/ssp", 0);
	if(fd < 0)
	{
		printf("Open /dev/ssp error!\n");
		return -1;
	}

	ret = ioctl(fd, SSP_WRITE_ALT, &value);

	close(fd);
	return 0;
}



HI_U16 u16Gamma[257];


void setgamma1()
{
	u16Gamma[0]=0;
	u16Gamma[1]=80;
	u16Gamma[2]=158;
	u16Gamma[3]=235;
	u16Gamma[4]=310;
	u16Gamma[5]=384;
	u16Gamma[6]=453;
	u16Gamma[7]=514;
	u16Gamma[8]=566;
	u16Gamma[9]=611;
	u16Gamma[10]=648;
	u16Gamma[11]=683;
	u16Gamma[12]=717;
	u16Gamma[13]=747;
	u16Gamma[14]=774;
	u16Gamma[15]=802;
	u16Gamma[16]=827;
	u16Gamma[17]=851;
	u16Gamma[18]=874;
	u16Gamma[19]=894;
	u16Gamma[20]=918;
	u16Gamma[21]=946;
	u16Gamma[22]=974;
	u16Gamma[23]=1005;
	u16Gamma[24]=1037;
	u16Gamma[25]=1070;
	u16Gamma[26]=1106;
	u16Gamma[27]=1139;
	u16Gamma[28]=1171;
	u16Gamma[29]=1200;
	u16Gamma[30]=1232;
	u16Gamma[31]=1259;
	u16Gamma[32]=1291;
	u16Gamma[33]=1317;
	u16Gamma[34]=1347;
	u16Gamma[35]=1376;
	u16Gamma[36]=1403;
	u16Gamma[37]=1427;
	u16Gamma[38]=1454;
	u16Gamma[39]=1478;
	u16Gamma[40]=1506;
	u16Gamma[41]=1530;
	u16Gamma[42]=1557;
	u16Gamma[43]=1579;
	u16Gamma[44]=1599;
	u16Gamma[45]=1619;
	u16Gamma[46]=1639;
	u16Gamma[47]=1659;
	u16Gamma[48]=1679;
	u16Gamma[49]=1699;
	u16Gamma[50]=1719;
	u16Gamma[51]=1739;
	u16Gamma[52]=1759;
	u16Gamma[53]=1779;
	u16Gamma[54]=1799;
	u16Gamma[55]=1819;
	u16Gamma[56]=1839;
	u16Gamma[57]=1859;
	u16Gamma[58]=1878;
	u16Gamma[59]=1898;
	u16Gamma[60]=1918;
	u16Gamma[61]=1938;
	u16Gamma[62]=1957;
	u16Gamma[63]=1977;
	u16Gamma[64]=1996;
	u16Gamma[65]=2015;
	u16Gamma[66]=2034;
	u16Gamma[67]=2050;
	u16Gamma[68]=2066;
	u16Gamma[69]=2082;
	u16Gamma[70]=2099;
	u16Gamma[71]=2115;
	u16Gamma[72]=2132;
	u16Gamma[73]=2148;
	u16Gamma[74]=2165;
	u16Gamma[75]=2181;
	u16Gamma[76]=2198;
	u16Gamma[77]=2214;
	u16Gamma[78]=2230;
	u16Gamma[79]=2246;
	u16Gamma[80]=2263;
	u16Gamma[81]=2279;
	u16Gamma[82]=2296;
	u16Gamma[83]=2312;
	u16Gamma[84]=2326;
	u16Gamma[85]=2342;
	u16Gamma[86]=2357;
	u16Gamma[87]=2373;
	u16Gamma[88]=2387;
	u16Gamma[89]=2403;
	u16Gamma[90]=2418;
	u16Gamma[91]=2434;
	u16Gamma[92]=2448;
	u16Gamma[93]=2464;
	u16Gamma[94]=2478;
	u16Gamma[95]=2491;
	u16Gamma[96]=2506;
	u16Gamma[97]=2518;
	u16Gamma[98]=2533;
	u16Gamma[99]=2546;
	u16Gamma[100]=2560;
	u16Gamma[101]=2573;
	u16Gamma[102]=2587;
	u16Gamma[103]=2600;
	u16Gamma[104]=2614;
	u16Gamma[105]=2627;
	u16Gamma[106]=2642;
	u16Gamma[107]=2654;
	u16Gamma[108]=2666;
	u16Gamma[109]=2678;
	u16Gamma[110]=2691;
	u16Gamma[111]=2702;
	u16Gamma[112]=2715;
	u16Gamma[113]=2726;
	u16Gamma[114]=2739;
	u16Gamma[115]=2750;
	u16Gamma[116]=2763;
	u16Gamma[117]=2774;
	u16Gamma[118]=2787;
	u16Gamma[119]=2800;
	u16Gamma[120]=2813;
	u16Gamma[121]=2826;
	u16Gamma[122]=2838;
	u16Gamma[123]=2851;
	u16Gamma[124]=2861;
	u16Gamma[125]=2872;
	u16Gamma[126]=2883;
	u16Gamma[127]=2893;
	u16Gamma[128]=2904;
	u16Gamma[129]=2915;
	u16Gamma[130]=2925;
	u16Gamma[131]=2935;
	u16Gamma[132]=2946;
	u16Gamma[133]=2956;
	u16Gamma[134]=2966;
	u16Gamma[135]=2977;
	u16Gamma[136]=2987;
	u16Gamma[137]=2997;
	u16Gamma[138]=3007;
	u16Gamma[139]=3017;
	u16Gamma[140]=3027;
	u16Gamma[141]=3037;
	u16Gamma[142]=3047;
	u16Gamma[143]=3057;
	u16Gamma[144]=3067;
	u16Gamma[145]=3077;
	u16Gamma[146]=3086;
	u16Gamma[147]=3096;
	u16Gamma[148]=3106;
	u16Gamma[149]=3116;
	u16Gamma[150]=3125;
	u16Gamma[151]=3135;
	u16Gamma[152]=3144;
	u16Gamma[153]=3154;
	u16Gamma[154]=3163;
	u16Gamma[155]=3173;
	u16Gamma[156]=3182;
	u16Gamma[157]=3192;
	u16Gamma[158]=3201;
	u16Gamma[159]=3210;
	u16Gamma[160]=3219;
	u16Gamma[161]=3229;
	u16Gamma[162]=3238;
	u16Gamma[163]=3247;
	u16Gamma[164]=3256;
	u16Gamma[165]=3265;
	u16Gamma[166]=3274;
	u16Gamma[167]=3283;
	u16Gamma[168]=3292;
	u16Gamma[169]=3301;
	u16Gamma[170]=3310;
	u16Gamma[171]=3319;
	u16Gamma[172]=3328;
	u16Gamma[173]=3337;
	u16Gamma[174]=3346;
	u16Gamma[175]=3355;
	u16Gamma[176]=3363;
	u16Gamma[177]=3372;
	u16Gamma[178]=3381;
	u16Gamma[179]=3390;
	u16Gamma[180]=3398;
	u16Gamma[181]=3407;
	u16Gamma[182]=3415;
	u16Gamma[183]=3424;
	u16Gamma[184]=3432;
	u16Gamma[185]=3441;
	u16Gamma[186]=3449;
	u16Gamma[187]=3458;
	u16Gamma[188]=3466;
	u16Gamma[189]=3475;
	u16Gamma[190]=3483;
	u16Gamma[191]=3492;
	u16Gamma[192]=3500;
	u16Gamma[193]=3508;
	u16Gamma[194]=3516;
	u16Gamma[195]=3525;
	u16Gamma[196]=3533;
	u16Gamma[197]=3541;
	u16Gamma[198]=3549;
	u16Gamma[199]=3557;
	u16Gamma[200]=3566;
	u16Gamma[201]=3574;
	u16Gamma[202]=3582;
	u16Gamma[203]=3590;
	u16Gamma[204]=3598;
	u16Gamma[205]=3606;
	u16Gamma[206]=3614;
	u16Gamma[207]=3622;
	u16Gamma[208]=3630;
	u16Gamma[209]=3638;
	u16Gamma[210]=3646;
	u16Gamma[211]=3654;
	u16Gamma[212]=3661;
	u16Gamma[213]=3669;
	u16Gamma[214]=3677;
	u16Gamma[215]=3685;
	u16Gamma[216]=3693;
	u16Gamma[217]=3700;
	u16Gamma[218]=3708;
	u16Gamma[219]=3716;
	u16Gamma[220]=3724;
	u16Gamma[221]=3731;
	u16Gamma[222]=3739;
	u16Gamma[223]=3747;
	u16Gamma[224]=3754;
	u16Gamma[225]=3762;
	u16Gamma[226]=3769;
	u16Gamma[227]=3777;
	u16Gamma[228]=3784;
	u16Gamma[229]=3792;
	u16Gamma[230]=3799;
	u16Gamma[231]=3807;
	u16Gamma[232]=3814;
	u16Gamma[233]=3822;
	u16Gamma[234]=3829;
	u16Gamma[235]=3837;
	u16Gamma[236]=3844;
	u16Gamma[237]=3851;
	u16Gamma[238]=3859;
	u16Gamma[239]=3866;
	u16Gamma[240]=3873;
	u16Gamma[241]=3881;
	u16Gamma[242]=3888;
	u16Gamma[243]=3895;
	u16Gamma[244]=3902;
	u16Gamma[245]=3910;
	u16Gamma[246]=3917;
	u16Gamma[247]=3924;
	u16Gamma[248]=3931;
	u16Gamma[249]=3938;
	u16Gamma[250]=3946;
	u16Gamma[251]=3953;
	u16Gamma[252]=3960;
	u16Gamma[253]=3967;
	u16Gamma[254]=3974;
	u16Gamma[255]=3981;
	u16Gamma[256]=3987;
}


void setgamma7()
{
	u16Gamma[0]=0;
	u16Gamma[1]=96;
	u16Gamma[2]=184;
	u16Gamma[3]=266;
	u16Gamma[4]=342;
	u16Gamma[5]=413;
	u16Gamma[6]=477;
	u16Gamma[7]=530;
	u16Gamma[8]=578;
	u16Gamma[9]=616;
	u16Gamma[10]=650;
	u16Gamma[11]=691;
	u16Gamma[12]=726;
	u16Gamma[13]=771;
	u16Gamma[14]=808;
	u16Gamma[15]=853;
	u16Gamma[16]=893;
	u16Gamma[17]=941;
	u16Gamma[18]=986;
	u16Gamma[19]=1034;
	u16Gamma[20]=1082;
	u16Gamma[21]=1133;
	u16Gamma[22]=1187;
	u16Gamma[23]=1245;
	u16Gamma[24]=1296;
	u16Gamma[25]=1338;
	u16Gamma[26]=1384;
	u16Gamma[27]=1424;
	u16Gamma[28]=1462;
	u16Gamma[29]=1496;
	u16Gamma[30]=1531;
	u16Gamma[31]=1562;
	u16Gamma[32]=1590;
	u16Gamma[33]=1618;
	u16Gamma[34]=1642;
	u16Gamma[35]=1666;
	u16Gamma[36]=1688;
	u16Gamma[37]=1710;
	u16Gamma[38]=1732;
	u16Gamma[39]=1754;
	u16Gamma[40]=1774;
	u16Gamma[41]=1795;
	u16Gamma[42]=1815;
	u16Gamma[43]=1835;
	u16Gamma[44]=1854;
	u16Gamma[45]=1873;
	u16Gamma[46]=1892;
	u16Gamma[47]=1911;
	u16Gamma[48]=1928;
	u16Gamma[49]=1945;
	u16Gamma[50]=1962;
	u16Gamma[51]=1978;
	u16Gamma[52]=1992;
	u16Gamma[53]=2008;
	u16Gamma[54]=2024;
	u16Gamma[55]=2040;
	u16Gamma[56]=2055;
	u16Gamma[57]=2070;
	u16Gamma[58]=2085;
	u16Gamma[59]=2100;
	u16Gamma[60]=2115;
	u16Gamma[61]=2129;
	u16Gamma[62]=2144;
	u16Gamma[63]=2158;
	u16Gamma[64]=2172;
	u16Gamma[65]=2186;
	u16Gamma[66]=2200;
	u16Gamma[67]=2213;
	u16Gamma[68]=2227;
	u16Gamma[69]=2240;
	u16Gamma[70]=2253;
	u16Gamma[71]=2266;
	u16Gamma[72]=2279;
	u16Gamma[73]=2292;
	u16Gamma[74]=2305;
	u16Gamma[75]=2317;
	u16Gamma[76]=2330;
	u16Gamma[77]=2343;
	u16Gamma[78]=2355;
	u16Gamma[79]=2368;
	u16Gamma[80]=2380;
	u16Gamma[81]=2392;
	u16Gamma[82]=2404;
	u16Gamma[83]=2416;
	u16Gamma[84]=2428;
	u16Gamma[85]=2439;
	u16Gamma[86]=2451;
	u16Gamma[87]=2462;
	u16Gamma[88]=2473;
	u16Gamma[89]=2484;
	u16Gamma[90]=2494;
	u16Gamma[91]=2505;
	u16Gamma[92]=2515;
	u16Gamma[93]=2526;
	u16Gamma[94]=2536;
	u16Gamma[95]=2546;
	u16Gamma[96]=2555;
	u16Gamma[97]=2565;
	u16Gamma[98]=2574;
	u16Gamma[99]=2584;
	u16Gamma[100]=2593;
	u16Gamma[101]=2602;
	u16Gamma[102]=2611;
	u16Gamma[103]=2619;
	u16Gamma[104]=2628;
	u16Gamma[105]=2636;
	u16Gamma[106]=2644;
	u16Gamma[107]=2652;
	u16Gamma[108]=2660;
	u16Gamma[109]=2668;
	u16Gamma[110]=2676;
	u16Gamma[111]=2683;
	u16Gamma[112]=2691;
	u16Gamma[113]=2698;
	u16Gamma[114]=2706;
	u16Gamma[115]=2713;
	u16Gamma[116]=2721;
	u16Gamma[117]=2728;
	u16Gamma[118]=2735;
	u16Gamma[119]=2742;
	u16Gamma[120]=2749;
	u16Gamma[121]=2756;
	u16Gamma[122]=2763;
	u16Gamma[123]=2769;
	u16Gamma[124]=2776;
	u16Gamma[125]=2782;
	u16Gamma[126]=2789;
	u16Gamma[127]=2795;
	u16Gamma[128]=2802;
	u16Gamma[129]=2808;
	u16Gamma[130]=2815;
	u16Gamma[131]=2821;
	u16Gamma[132]=2827;
	u16Gamma[133]=2834;
	u16Gamma[134]=2840;
	u16Gamma[135]=2847;
	u16Gamma[136]=2853;
	u16Gamma[137]=2859;
	u16Gamma[138]=2866;
	u16Gamma[139]=2872;
	u16Gamma[140]=2879;
	u16Gamma[141]=2885;
	u16Gamma[142]=2891;
	u16Gamma[143]=2898;
	u16Gamma[144]=2904;
	u16Gamma[145]=2911;
	u16Gamma[146]=2917;
	u16Gamma[147]=2923;
	u16Gamma[148]=2929;
	u16Gamma[149]=2935;
	u16Gamma[150]=2940;
	u16Gamma[151]=2946;
	u16Gamma[152]=2951;
	u16Gamma[153]=2957;
	u16Gamma[154]=2963;
	u16Gamma[155]=2968;
	u16Gamma[156]=2974;
	u16Gamma[157]=2979;
	u16Gamma[158]=2985;
	u16Gamma[159]=2991;
	u16Gamma[160]=2996;
	u16Gamma[161]=3002;
	u16Gamma[162]=3007;
	u16Gamma[163]=3013;
	u16Gamma[164]=3019;
	u16Gamma[165]=3024;
	u16Gamma[166]=3030;
	u16Gamma[167]=3035;
	u16Gamma[168]=3041;
	u16Gamma[169]=3047;
	u16Gamma[170]=3052;
	u16Gamma[171]=3058;
	u16Gamma[172]=3063;
	u16Gamma[173]=3069;
	u16Gamma[174]=3075;
	u16Gamma[175]=3080;
	u16Gamma[176]=3086;
	u16Gamma[177]=3091;
	u16Gamma[178]=3097;
	u16Gamma[179]=3103;
	u16Gamma[180]=3108;
	u16Gamma[181]=3114;
	u16Gamma[182]=3119;
	u16Gamma[183]=3125;
	u16Gamma[184]=3131;
	u16Gamma[185]=3136;
	u16Gamma[186]=3142;
	u16Gamma[187]=3147;
	u16Gamma[188]=3153;
	u16Gamma[189]=3159;
	u16Gamma[190]=3164;
	u16Gamma[191]=3170;
	u16Gamma[192]=3176;
	u16Gamma[193]=3182;
	u16Gamma[194]=3188;
	u16Gamma[195]=3195;
	u16Gamma[196]=3201;
	u16Gamma[197]=3208;
	u16Gamma[198]=3215;
	u16Gamma[199]=3222;
	u16Gamma[200]=3229;
	u16Gamma[201]=3236;
	u16Gamma[202]=3244;
	u16Gamma[203]=3252;
	u16Gamma[204]=3259;
	u16Gamma[205]=3267;
	u16Gamma[206]=3276;
	u16Gamma[207]=3284;
	u16Gamma[208]=3292;
	u16Gamma[209]=3301;
	u16Gamma[210]=3310;
	u16Gamma[211]=3319;
	u16Gamma[212]=3328;
	u16Gamma[213]=3337;
	u16Gamma[214]=3347;
	u16Gamma[215]=3356;
	u16Gamma[216]=3366;
	u16Gamma[217]=3376;
	u16Gamma[218]=3386;
	u16Gamma[219]=3397;
	u16Gamma[220]=3408;
	u16Gamma[221]=3420;
	u16Gamma[222]=3431;
	u16Gamma[223]=3443;
	u16Gamma[224]=3456;
	u16Gamma[225]=3468;
	u16Gamma[226]=3481;
	u16Gamma[227]=3494;
	u16Gamma[228]=3508;
	u16Gamma[229]=3522;
	u16Gamma[230]=3536;
	u16Gamma[231]=3551;
	u16Gamma[232]=3566;
	u16Gamma[233]=3581;
	u16Gamma[234]=3596;
	u16Gamma[235]=3612;
	u16Gamma[236]=3628;
	u16Gamma[237]=3645;
	u16Gamma[238]=3662;
	u16Gamma[239]=3679;
	u16Gamma[240]=3697;
	u16Gamma[241]=3716;
	u16Gamma[242]=3734;
	u16Gamma[243]=3754;
	u16Gamma[244]=3774;
	u16Gamma[245]=3794;
	u16Gamma[246]=3815;
	u16Gamma[247]=3836;
	u16Gamma[248]=3858;
	u16Gamma[249]=3880;
	u16Gamma[250]=3903;
	u16Gamma[251]=3926;
	u16Gamma[252]=3950;
	u16Gamma[253]=3974;
	u16Gamma[254]=3998;
	u16Gamma[255]=4024;
	u16Gamma[256]=4049;
}

const HI_U16 gs_Gamma[4][GAMMA_NODE_NUMBER] = {
	{
		0 ,120 ,220 ,310 ,390 ,470 ,540 ,610 ,670 ,730 ,786 ,842 ,894 ,944 ,994 ,1050,1096,
		1138,1178,1218,1254,1280,1314,1346,1378,1408,1438,1467,1493,1519,1543,1568,1592,
		1615,1638,1661,1683,1705,1726,1748,1769,1789,1810,1830,1849,1869,1888,1907,1926,
		1945,1963,1981,1999,2017,2034,2052,2069,2086,2102,2119,2136,2152,2168,2184,2200,
		2216,2231,2247,2262,2277,2292,2307,2322,2337,2351,2366,2380,2394,2408,2422,2436,
		2450,2464,2477,2491,2504,2518,2531,2544,2557,2570,2583,2596,2609,2621,2634,2646,
		2659,2671,2683,2696,2708,2720,2732,2744,2756,2767,2779,2791,2802,2814,2825,2837,
		2848,2859,2871,2882,2893,2904,2915,2926,2937,2948,2959,2969,2980,2991,3001,3012,
		3023,3033,3043,3054,3064,3074,3085,3095,3105,3115,3125,3135,3145,3155,3165,3175,
		3185,3194,3204,3214,3224,3233,3243,3252,3262,3271,3281,3290,3300,3309,3318,3327,
		3337,3346,3355,3364,3373,3382,3391,3400,3409,3418,3427,3436,3445,3454,3463,3471,
		3480,3489,3498,3506,3515,3523,3532,3540,3549,3557,3566,3574,3583,3591,3600,3608,
		3616,3624,3633,3641,3649,3657,3665,3674,3682,3690,3698,3706,3714,3722,3730,3738,
		3746,3754,3762,3769,3777,3785,3793,3801,3808,3816,3824,3832,3839,3847,3855,3862,
		3870,3877,3885,3892,3900,3907,3915,3922,3930,3937,3945,3952,3959,3967,3974,3981,
		3989,3996,4003,4010,4018,4025,4032,4039,4046,4054,4061,4068,4075,4082,4089,4095,
	},
	{
		0, 54, 106, 158, 209, 259, 308, 356, 403, 450, 495, 540, 584, 628, 670, 713, 754, 795,
		835, 874, 913, 951, 989,1026,1062,1098,1133,1168,1203,1236,1270,1303,
		1335,1367,1398,1429,1460,1490,1520,1549,1578,1607,1635,1663,1690,1717,1744,1770,
		1796,1822,1848,1873,1897,1922,1946,1970,1993,2017,2040,2062,2085,2107,2129,2150,
		2172,2193,2214,2235,2255,2275,2295,2315,2335,2354,2373,2392,2411,2429,2447,2465,
		2483,2501,2519,2536,2553,2570,2587,2603,2620,2636,2652,2668,2684,2700,2715,2731,
		2746,2761,2776,2790,2805,2819,2834,2848,2862,2876,2890,2903,2917,2930,2944,2957,
		2970,2983,2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152,
		3164,3175,3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323,
		3333,3343,3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473,
		3482,3490,3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606,
		3614,3621,3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725,
		3732,3739,3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831,
		3838,3844,3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928,
		3933,3939,3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015,
		4020,4026,4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095,
		4095,
	},
	{
		0, 27, 60, 100, 140, 178, 216, 242, 276, 312, 346, 380, 412, 444, 476, 508,
		540, 572, 604, 636, 667, 698, 729, 760, 791, 822, 853, 884, 915, 945, 975, 1005,
		1035, 1065, 1095, 1125, 1155, 1185, 1215, 1245, 1275, 1305, 1335, 1365, 1395, 1425,
		1455, 1485,
		1515, 1544, 1573, 1602, 1631, 1660, 1689, 1718, 1746, 1774, 1802, 1830, 1858, 1886,
		1914, 1942,
		1970, 1998, 2026, 2054, 2082, 2110, 2136, 2162, 2186, 2220, 2244, 2268, 2292, 2316,
		2340, 2362,
		2384, 2406, 2428, 2448, 2468, 2488, 2508, 2528, 2548, 2568, 2588, 2608, 2628, 2648,
		2668, 2688,
		2708, 2728, 2748, 2768, 2788, 2808, 2828, 2846, 2862, 2876, 2890, 2903, 2917, 2930,
		2944, 2957,
		2970, 2983, 2996, 3008, 3021, 3033, 3046, 3058, 3070, 3082, 3094, 3106, 3118, 3129,
		3141, 3152,3164, 3175, 3186, 3197, 3208, 3219, 3230, 3240, 3251, 3262, 3272, 3282, 3293, 3303,
		3313, 3323,
		3333, 3343, 3352, 3362, 3372, 3381, 3391, 3400, 3410, 3419, 3428, 3437, 3446, 3455,
		3464, 3473,
		3482, 3490, 3499, 3508, 3516, 3525, 3533, 3541, 3550, 3558, 3566, 3574, 3582, 3590,
		3598, 3606,
		3614, 3621, 3629, 3637, 3644, 3652, 3660, 3667, 3674, 3682, 3689, 3696, 3703, 3711,
		3718, 3725,
		3732, 3739, 3746, 3752, 3759, 3766, 3773, 3779, 3786, 3793, 3799, 3806, 3812, 3819,
		3825, 3831,
		3838, 3844, 3850, 3856, 3863, 3869, 3875, 3881, 3887, 3893, 3899, 3905, 3910, 3916,
		3922, 3928,
		3933, 3939, 3945, 3950, 3956, 3962, 3967, 3973, 3978, 3983, 3989, 3994, 3999, 4005,
		4010, 4015,
		4020, 4026, 4031, 4036, 4041, 4046, 4051, 4056, 4061, 4066, 4071, 4076, 4081, 4085,
		4090, 4095, 4095
	},
	{
		0x0,0x11,0x2b,0x4d,0x75,0xa1,0xd0,0xff,0x12f,0x15e,0x18d,0x1bc,0x1ea,0x217,0x244,0x271,
		0x29d,0x2c9,0x2f4,0x31f,0x349,0x373,0x39d,0x3c6,0x3ef,0x417,0x43f,0x466,0x48d,0x4b4,0x4da,0x500,
		0x525,0x54a,0x56f,0x593,0x5b7,0x5da,0x5fd,0x620,0x642,0x664,0x685,0x6a7,0x6c7,0x6e7,0x707,0x727,
		0x746,0x765,0x783,0x7a1,0x7bf,0x7dc,0x7f9,0x816,0x832,0x84e,0x869,0x884,0x89f,0x8b9,0x8d3,0x8ed,
		0x907,0x920,0x938,0x951,0x969,0x980,0x998,0x9af,0x9c6,0x9dc,0x9f2,0xa08,0xa1e,0xa33,0xa48,0xa5d,
		0xa71,0xa85,0xa99,0xaad,0xac0,0xad3,0xae6,0xaf9,0xb0b,0xb1d,0xb2f,0xb40,0xb52,0xb63,0xb74,0xb84,
		0xb95,0xba5,0xbb5,0xbc5,0xbd5,0xbe4,0xbf3,0xc02,0xc11,0xc20,0xc2e,0xc3c,0xc4b,0xc58,0xc66,0xc74,
		0xc81,0xc8f,0xc9c,0xca9,0xcb5,0xcc2,0xccf,0xcdb,0xce7,0xcf4,0xd00,0xd0b,0xd17,0xd23,0xd2e,0xd3a,
		0xd45,0xd50,0xd5b,0xd66,0xd71,0xd7b,0xd86,0xd90,0xd9b,0xda5,0xdaf,0xdb9,0xdc3,0xdcc,0xdd6,0xddf,
		0xde9,0xdf2,0xdfb,0xe04,0xe0d,0xe16,0xe1f,0xe27,0xe30,0xe38,0xe40,0xe48,0xe50,0xe58,0xe60,0xe68,
		0xe70,0xe77,0xe7f,0xe86,0xe8d,0xe95,0xe9c,0xea3,0xeaa,0xeb0,0xeb7,0xebe,0xec4,0xecb,0xed1,0xed7,
		0xedd,0xee4,0xeea,0xef0,0xef5,0xefb,0xf01,0xf06,0xf0c,0xf11,0xf17,0xf1c,0xf21,0xf26,0xf2b,0xf30,
		0xf35,0xf3a,0xf3f,0xf44,0xf48,0xf4d,0xf51,0xf56,0xf5a,0xf5e,0xf63,0xf67,0xf6b,0xf6f,0xf73,0xf77,
		0xf7b,0xf7f,0xf82,0xf86,0xf8a,0xf8d,0xf91,0xf94,0xf98,0xf9b,0xf9e,0xfa2,0xfa5,0xfa8,0xfab,0xfae,
		0xfb1,0xfb4,0xfb7,0xfba,0xfbd,0xfc0,0xfc2,0xfc5,0xfc8,0xfca,0xfcd,0xfd0,0xfd2,0xfd5,0xfd7,0xfda,
		0xfdc,0xfde,0xfe1,0xfe3,0xfe5,0xfe7,0xfea,0xfec,0xfee,0xff0,0xff2,0xff4,0xff6,0xff8,0xffb,0xffd,
		0xfff
	}
};


SDK_ISP_SENSOR_MODEL_t g_sensor_type = SDK_ISP_SENSOR_MODEL_APINA_AR0130;//default
static uint8_t gs_ircut_auto_switch_enable = HI_TRUE;

#define AR0130_CHECK_DATA (0x2402)
#define OV9712_CHECK_DATA_MSB (0x97)
#define OV9712_CHECK_DATA_LSB (0x11)
#define SOIH22_CHECK_DATA_MSB (0xa0)
#define SOIH22_CHECK_DATA_LSB (0x22)

SDK_API_t SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency)
{
	ISP_ANTIFLICKER_S pstAntiflicker;
	pstAntiflicker.bEnable = bEnable;
	pstAntiflicker.u8Frequency = frequency;
	pstAntiflicker.enMode = ISP_ANTIFLICKER_MODE_1;
	printf("%s---%d:%d\r\n", __FUNCTION__, frequency, bEnable);
	SOC_CHECK(HI_MPI_ISP_SetAntiFlickerAttr(&pstAntiflicker));
	//printf("%s-%d:%d/%d\r\n", __FUNCTION__, __LINE__, pstAntiflicker.bEnable, pstAntiflicker.u8Frequency);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_sensor_check()
{
	unsigned int ret_data1 = 0;
	unsigned int ret_data2 = 0;
	do{
		//ar0130
		ar0130_i2c_read(0x3000, &ret_data1);
		if(ret_data1 == AR0130_CHECK_DATA){
			g_sensor_type = SDK_ISP_SENSOR_MODEL_APINA_AR0130;
			sdk_sys->write_reg(0x20030030, 0x5); //set sensor clock
			break;
		}

		//ov9712
		ov9712_i2c_read(0x0a, &ret_data1);
		ov9712_i2c_read(0x0b, &ret_data2);
		if(ret_data1 == OV9712_CHECK_DATA_MSB && ret_data2 == OV9712_CHECK_DATA_LSB){
			g_sensor_type = SDK_ISP_SENSOR_MODEL_OV_OV9712;
			sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
			break;
		}
		soih22_i2c_read(0x0a, &ret_data1);
		soih22_i2c_read(0x0b, &ret_data2);
		if(ret_data1 == SOIH22_CHECK_DATA_MSB && ret_data2 == SOIH22_CHECK_DATA_LSB){
			g_sensor_type = SDK_ISP_SENSOR_MODEL_SOI_H22;
			sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
			break;
		}

		{
			g_sensor_type = SDK_ISP_SENSOR_MODEL_SONY_IMX122;
			sdk_sys->write_reg(0x200f000c, 0x1);
			sdk_sys->write_reg(0x200f0010, 0x1);
			sdk_sys->write_reg(0x200f0014, 0x1);
			sdk_sys->write_reg(0x20030030, 0x6);
		}	
	}while(0);
	
	return SDK_SUCCESS;
}


SDK_API_t SDK_ISP_ircut_auto_switch(int vin, uint8_t type)//1:software   0: hardware 
{
	static uint32_t old_satuation = 0;

	//uint32_t gpio_status_cur = isp_gpio_pin_read(IRCUT_GPIO_GROUP, IRCUT_GPIO_PIN);
	uint32_t gpio_status_cur;

	//SOC_CHECK(HI_MPI_ISP_SetSaturation(0));
	//printf("GPIO_PIN:%x/%x\r\n", isp_gpio_pin_read(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN), isp_gpio_pin_read(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN));
	if(gs_ircut_auto_switch_enable){
		if(!type){//hardware detect
			gpio_status_cur= isp_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN);
		}else{//software detect
			gpio_status_cur = sdk_isp_calculate_exposure(gs_gpio_status_old);
		}
		
		if(gs_gpio_status_old != gpio_status_cur){
			gs_gpio_status_old = gpio_status_cur;
		
			if(!gpio_status_cur){			
				isp_ircut_switch(0);
			}else{			
				isp_ircut_switch(1);
			}
		}
	}else{
		//do nothing
	}
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_mirror(int vin, bool mirror)
{
	VI_CHN_ATTR_S vi_chn_attr;
	SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
	vi_chn_attr.bMirror = mirror ? HI_TRUE : HI_FALSE;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(vin, &vi_chn_attr));
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_flip(int vin, bool flip)
{
	VI_CHN_ATTR_S vi_chn_attr;
	SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
	vi_chn_attr.bFlip = flip ? HI_TRUE : HI_FALSE;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(vin, &vi_chn_attr));
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_rotate(int vin, int rotate_n)
{
	ROTATE_E enRotate = ROTATE_NONE;
	switch(rotate_n)	{
	case SDK_ISP_ROTATE_0: enRotate = ROTATE_NONE; break;
	case SDK_ISP_ROTATE_90: enRotate = ROTATE_90; break;
	case SDK_ISP_ROTATE_180: enRotate = ROTATE_180; break;
	case SDK_ISP_ROTATE_270: enRotate = ROTATE_270; break;
	default:
		return SDK_FAILURE;
	}
	SOC_CHECK(HI_MPI_VI_SetRotate(vin, enRotate));
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_saturation(int vin, uint16_t val)
{
#if 0
	HI_U8 pu8Value;
	HI_MPI_ISP_SetSaturation(val);
	printf("saturation set:%d\r\n", val);
#else
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32SatuVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("saturation set:%d\r\n", val);
#endif
}

SDK_API_t SDK_ISP_get_saturation(int vin, uint16_t *val)
{
/*
	HI_U32 pu8Value;
	HI_MPI_ISP_GetSaturation(&pu8Value);
	printf("saturation get:%d\r\n", pu8Value);
	*val = (uint16_t)pu8Value;
*/
	return SDK_SUCCESS;
}


SDK_API_t SDK_ISP_set_contrast(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32ContrVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("contrast set:%d\r\n", val);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_brightness(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32LumaVal= val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("brightness set:%d\r\n", val);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_hue(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32HueVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("hue set:%d\r\n", val);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_src_framerate(unsigned int framerate)
{
  VI_CHN_ATTR_S vi_chn_attr;
  ISP_IMAGE_ATTR_S isp_image_attr;
  SOC_CHECK(HI_MPI_ISP_GetImageAttr(&isp_image_attr));
  isp_image_attr.u16FrameRate = framerate;
  SOC_CHECK(HI_MPI_ISP_SetImageAttr(&isp_image_attr));
   return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_get_sharpen(uint8_t *val)
{
	ISP_SHARPEN_ATTR_S SharpenAttr;
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(&SharpenAttr));
	int iso = isp_get_iso();
	if(iso < 200){
		*val = SharpenAttr.u8SharpenAltUd[0];
	}else if(iso < 400){
		*val = SharpenAttr.u8SharpenAltUd[1];
	}else if(iso < 800){
		*val = SharpenAttr.u8SharpenAltUd[2];
	}else if(iso < 1600){
		*val = SharpenAttr.u8SharpenAltUd[3];
	}else if(iso < 3200){
		*val = SharpenAttr.u8SharpenAltUd[4];
	}else if(iso < 6400){
		*val = SharpenAttr.u8SharpenAltUd[5];
	}else if(iso < 12800){
		*val = SharpenAttr.u8SharpenAltUd[6];
	}else{
		*val = SharpenAttr.u8SharpenAltUd[7];
	};
	return SDK_SUCCESS;
}


SDK_API_t SDK_ISP_set_sharpen(uint8_t val)
{
	ISP_SHARPEN_ATTR_S SharpenAttr;
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(&SharpenAttr));

	SharpenAttr.bEnable = HI_TRUE;
	/*SharpenAttr.bManualEnable = HI_TRUE;
	if(SharpenAttr.u8StrengthMin <= val){
		SharpenAttr.u8StrengthTarget = val;
	}else{
		SharpenAttr.u8StrengthTarget = SharpenAttr.u8StrengthMin;
	}*/
	//SharpenAttr.u8StrengthMin = 0;
	int iso = isp_get_iso();
	if(iso < 400){
		SharpenAttr.u8SharpenAltUd[0] = val;
		SharpenAttr.u8SharpenAltUd[1] = val;
		printf("sharpen01-%d: %d\r\n", iso, val);
	}else if(iso < 800){
		SharpenAttr.u8SharpenAltUd[2] = val;
		printf("sharpen02-%d: %d\r\n", iso, val);
	}else if(iso < 1600){
		SharpenAttr.u8SharpenAltUd[3] = val;
		printf("sharpen03-%d: %d\r\n", iso, val);
	}else if(iso < 3200){
		SharpenAttr.u8SharpenAltUd[4] = val;
		printf("sharpen04-%d: %d\r\n", iso, val);
	}else if(iso < 6400){
		SharpenAttr.u8SharpenAltUd[5] = val;
		printf("sharpen05-%d: %d\r\n", iso, val);
	}else if(iso < 12800){
		SharpenAttr.u8SharpenAltUd[6] = val;
		printf("sharpen06-%d: %d\r\n", iso, val);
	}else{
		SharpenAttr.u8SharpenAltUd[7] = val;
		printf("sharpen07-%d: %d\r\n", iso, val);
	}
	
	SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(&SharpenAttr));
 	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_scene_mode(uint32_t mode)
{
	ISP_DRC_ATTR_S DRCAttr;
	ISP_GAMMA_TABLE_S GammaAttr;
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&GammaAttr)); 
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_SCENE_MODE_AUTO:
			DRCAttr.bDRCEnable = HI_TRUE;
			switch(g_sensor_type){
				default:
				case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
					break;
				case SDK_ISP_SENSOR_MODEL_OV_OV9712:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));		
					break;
				case SDK_ISP_SENSOR_MODEL_SOI_H22:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					setgamma7();
					memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					break;
			} 
			break;
		case ISP_SCENE_MODE_INDOOR:
			DRCAttr.bDRCEnable = HI_TRUE;
			switch(g_sensor_type){
				default:
				case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[1], sizeof(gs_Gamma[1]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
					break;
				case SDK_ISP_SENSOR_MODEL_OV_OV9712:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));		
					break;
				case SDK_ISP_SENSOR_MODEL_SOI_H22:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					setgamma7();
					memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					break;
					break;
			}
			break;
		case ISP_SCENE_MODE_OUTDOOR:
			DRCAttr.bDRCEnable = HI_FALSE;
			switch(g_sensor_type){
				default:
				case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
					break;
				case SDK_ISP_SENSOR_MODEL_OV_OV9712:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));		
					break;
				case SDK_ISP_SENSOR_MODEL_SOI_H22:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					setgamma7();
					memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					break;
			}
			break;
	}
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(&DRCAttr));
	SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr)); 

	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_WB_mode(uint32_t mode)
{
	ISP_OP_TYPE_E enWBType;
	ISP_AWB_ATTR_S AWBAttr;
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_SCENE_MODE_AUTO:
			break;
		case ISP_SCENE_MODE_INDOOR:

			break;
		case ISP_SCENE_MODE_OUTDOOR:

			break;
	}
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_ircut_control_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_ircut_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_IRCUT_MODE_AUTO:
			gs_ircut_auto_switch_enable = HI_TRUE;
			break;
		case ISP_IRCUT_MODE_DAYLIGHT:
			gs_ircut_auto_switch_enable = HI_FALSE;
			isp_ircut_switch(0);
			break;
		case ISP_IRCUT_MODE_NIGHT:
			gs_ircut_auto_switch_enable = HI_FALSE;
			isp_ircut_switch(1);
			break;
	}
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_WDR_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	ISP_DRC_ATTR_S DRCAttr;
	switch(g_sensor_type){
		default:
		case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
		case SDK_ISP_SENSOR_MODEL_OV_OV9712:
		case SDK_ISP_SENSOR_MODEL_SOI_H22:
			HI_MPI_ISP_GetDRCAttr(&DRCAttr);
			DRCAttr.bDRCEnable = bEnable;
			HI_MPI_ISP_SetDRCAttr(&DRCAttr);
		break;
	}
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_WDR_strength(uint8_t val)
{
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_exposure_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	ISP_AE_ATTR_S AEAttr;
	HI_MPI_ISP_GetAEAttr(&AEAttr);
	switch(mode){
		default:
		case ISP_EXPOSURE_MODE_AUTO:
			switch(g_sensor_type){
				default:
				case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
					AEAttr.u8ExpCompensation = 0x80;
					break;
				case SDK_ISP_SENSOR_MODEL_OV_OV9712:
					AEAttr.u8ExpCompensation = 0x78;
					break;
				case SDK_ISP_SENSOR_MODEL_SOI_H22:
					AEAttr.u8ExpCompensation = 0x70;
					break;
			}			
			break;
			
		case ISP_EXPOSURE_MODE_BRIGHT:
			switch(g_sensor_type){
				default:
				case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
					AEAttr.u8ExpCompensation = 0xb8;
					break;
				case SDK_ISP_SENSOR_MODEL_OV_OV9712:
					AEAttr.u8ExpCompensation = 0xb0;
					break;
				case SDK_ISP_SENSOR_MODEL_SOI_H22:
					AEAttr.u8ExpCompensation = 0xa8;
					break;
			}			
			break;

		case ISP_EXPOSURE_MODE_DARK:
			switch(g_sensor_type){
				default:
				case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
					AEAttr.u8ExpCompensation = 0x48;
					break;
				case SDK_ISP_SENSOR_MODEL_OV_OV9712:
					AEAttr.u8ExpCompensation = 0x40;
					break;
				case SDK_ISP_SENSOR_MODEL_SOI_H22:
					AEAttr.u8ExpCompensation = 0x38;
					break;
			}			
			break;
	}
	HI_MPI_ISP_SetAEAttr(&AEAttr);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_AEcompensation(uint8_t val)
{
	ISP_AE_ATTR_S AEAttr;
	SOC_CHECK(HI_MPI_ISP_GetAEAttr(&AEAttr));
	AEAttr.u8ExpCompensation = val;
	SOC_CHECK(HI_MPI_ISP_SetAEAttr(&AEAttr));
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_denoise_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	VPSS_GRP_PARAM_S VpssParam;
	HI_MPI_VPSS_GetGrpParam(0, &VpssParam);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_get_denoise_strength(uint8_t *val)
{
	ISP_DENOISE_ATTR_S DenoiseAttr;
	HI_MPI_ISP_GetDenoiseAttr(&DenoiseAttr);
	//DenoiseAttr.bManualEnable = 1;
	int iso = isp_get_iso();
	if(iso < 200){
		*val = DenoiseAttr.u8SnrThresh[0];
	}else if(iso < 800){
		*val = DenoiseAttr.u8SnrThresh[1];
	}else if(iso < 800){
		*val = DenoiseAttr.u8SnrThresh[2];
	}else if(iso < 1600){
		*val = DenoiseAttr.u8SnrThresh[3];
	}else if(iso < 3200){
		*val = DenoiseAttr.u8SnrThresh[4];
	}else if(iso < 6400){
		*val = DenoiseAttr.u8SnrThresh[5];
	}else if(iso < 12800){
		*val = DenoiseAttr.u8SnrThresh[6];
	}else{
		*val = DenoiseAttr.u8SnrThresh[7];
	}
	return SDK_SUCCESS;
}


SDK_API_t SDK_ISP_set_denoise_strength(uint8_t val)
{
	ISP_DENOISE_ATTR_S DenoiseAttr;
	HI_MPI_ISP_GetDenoiseAttr(&DenoiseAttr);
	//DenoiseAttr.bManualEnable = 1;
	int iso = isp_get_iso();
	if(iso < 400){
		DenoiseAttr.u8SnrThresh[0] = val;
		DenoiseAttr.u8SnrThresh[1] = val;
		printf("denoise01-%d: %d\r\n", iso, val);
	}else if(iso < 800){
		DenoiseAttr.u8SnrThresh[2] = val;
		printf("denoise2-%d: %d\r\n", iso, val);
	}else if(iso < 1600){
		DenoiseAttr.u8SnrThresh[3] = val;
		printf("denoise3-%d: %d\r\n", iso, val);
	}else if(iso < 3200){
		DenoiseAttr.u8SnrThresh[4] = val;
		printf("denoise4 %d: %d\r\n", iso, val);
	}else if(iso < 6400){
		DenoiseAttr.u8SnrThresh[5] = val;
		printf("denoise5 %d: %d\r\n", iso, val);
	}else if(iso < 12800){
		DenoiseAttr.u8SnrThresh[6] = val;
		printf("denoise6 %d: %d\r\n", iso, val);
	}else{
		DenoiseAttr.u8SnrThresh[7] = val;
		printf("denoise7 %d: %d\r\n", iso, val);
	}
	HI_MPI_ISP_SetDenoiseAttr(&DenoiseAttr);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	ISP_ANTIFOG_S AntiFog;
	AntiFog.bEnable = bEnable;
	HI_MPI_ISP_SetAntiFogAttr(&AntiFog);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	if(bEnable){
		gs_ircut_auto_switch_enable = HI_FALSE;
		switch(g_sensor_type){
			case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
				{
					
				}
				break;
			case SDK_ISP_SENSOR_MODEL_OV_OV9712:
				{

				}
				break;
			case SDK_ISP_SENSOR_MODEL_SOI_H22:
				{

				}
				break;
		}
	}else{
		gs_ircut_auto_switch_enable = HI_TRUE;
		switch(g_sensor_type){
			case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
				{
					
				}
				break;
			case SDK_ISP_SENSOR_MODEL_OV_OV9712:
				{

				}
				break;
			case SDK_ISP_SENSOR_MODEL_SOI_H22:
				{

				}
				break;
		}
	}
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_advance_gamma_table(uint8_t val)
{
	ISP_GAMMA_TABLE_S GammaAttr;
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&GammaAttr)); 
	memcpy(GammaAttr.u16Gamma, gs_Gamma[val], sizeof(gs_Gamma[val]));
	SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr)); 
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable)
{
	ISP_DP_ATTR_S DPAttr;
	HI_MPI_ISP_GetDefectPixelAttr(&DPAttr);
	printf("DPAttr.bEnableStatic:%d\r\n", DPAttr.bEnableStatic);
	printf("DPAttr.bEnableDynamic:%d\r\n", DPAttr.bEnableDynamic);
	printf("DPAttr.bEnableDetect:%d\r\n", DPAttr.bEnableDetect);
	HI_MPI_ISP_SetDefectPixelAttr(&DPAttr);
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_set_isp_default_value(int sensor_type, int mode)////mode  0:daytime   1:night
{
	ISP_GAMMA_TABLE_S GammaAttr;
	HI_MPI_ISP_GetGammaTable(&GammaAttr); 
	
	switch(sensor_type){
		default:
		case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
			{
				if(!mode){//daytime
					ISP_AE_ATTR_S AEAttr;
					HI_MPI_ISP_GetAEAttr(&AEAttr);
					AEAttr.s16ExpTolerance = 0x18;
					HI_MPI_ISP_SetAEAttr(&AEAttr);

					ISP_AWB_ATTR_S AWBAttr;
					SOC_CHECK(HI_MPI_ISP_GetAWBAttr(&AWBAttr));
					AWBAttr.u8RGStrength = 0x71;
					AWBAttr.u8BGStrength = 0x77;
					SOC_CHECK(HI_MPI_ISP_SetAWBAttr(&AWBAttr));
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
				}else{//night
				
				}
			}
		break;
		case SDK_ISP_SENSOR_MODEL_OV_OV9712:
			{
				if(!mode){//daytime
					ISP_AE_ATTR_S AEAttr;
					HI_MPI_ISP_GetAEAttr(&AEAttr);
					AEAttr.s16ExpTolerance = 0x18;
					AEAttr.enFrameEndUpdateMode = 0x2;
					HI_MPI_ISP_SetAEAttr(&AEAttr);

					ISP_AWB_ATTR_S AWBAttr;
					SOC_CHECK(HI_MPI_ISP_GetAWBAttr(&AWBAttr));
					AWBAttr.u8RGStrength = 0x74;
					AWBAttr.u8BGStrength = 0x78;
					SOC_CHECK(HI_MPI_ISP_SetAWBAttr(&AWBAttr));
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));		
				}else{//night
				
				}
			}
		break;
		case SDK_ISP_SENSOR_MODEL_SOI_H22:
			{			
				if(!mode){
					ISP_SHADING_ATTR_S ShadingAttr;
					SOC_CHECK(HI_MPI_ISP_GetShadingAttr(&ShadingAttr));
					ShadingAttr.Enable = HI_FALSE;
					SOC_CHECK(HI_MPI_ISP_SetShadingAttr(&ShadingAttr));
					
					ISP_AWB_ATTR_S AWBAttr;
					SOC_CHECK(HI_MPI_ISP_GetAWBAttr(&AWBAttr));
					AWBAttr.u8RGStrength = 0x54;
					AWBAttr.u8BGStrength = 0x5f;
					SOC_CHECK(HI_MPI_ISP_SetAWBAttr(&AWBAttr));

					ISP_AE_ATTR_S AEAttr;
					SOC_CHECK(HI_MPI_ISP_GetAEAttr(&AEAttr));
					AEAttr.s16ExpTolerance = 0x18;
					SOC_CHECK(HI_MPI_ISP_SetAEAttr(&AEAttr));
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					setgamma7();
					memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					//memcpy(GammaAttr.u16Gamma, gs_Gamma[1], sizeof(gs_Gamma[1]));		
				}else{//night

				}
			}
		break;
		case SDK_ISP_SENSOR_MODEL_SONY_IMX122:
			{

			}
		break;
	}
	HI_MPI_ISP_SetGammaTable(&GammaAttr); 
	return SDK_SUCCESS;
}

/*typedef struct _isp_sensor_default_value
{
	uint16_t u16Gamma[2][257];
	uint8_t u8AEcompensation;
	uint8_t	u8AWBRGstrength;
	uint8_t u8AWBBGstrength;
	uint8_t u8SharpenD[2][8];
	uint8_t u8SharpenUD[2][8];
	
}ISP_SENSOR_default_value_t;*/

#define SET_VI_DEV_ATTR_AR0130(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\ 
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\ 
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\ 
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}\	

#define SET_VI_DEV_ATTR_OV9712(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\ 
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\ 
			info.au32CompMask[0] = 0xFFC00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\ 
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}\	

#define SET_VI_DEV_ATTR_SOIH22(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\ 
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\ 
			info.au32CompMask[0] = 0xFFC00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\ 
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}\	

#define SET_VI_DEV_ATTR_IMX122(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\ 
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\ 
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\ 
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}\	


SDK_API_t SDK_ISP_init()
{
	int ret = 0;
	pthread_t isp_tid = NULL;
	VI_DEV_ATTR_S vi_dev_attr_720p_30fps;
	VI_CHN_ATTR_S vin_chn_attr;
	VPSS_GRP_ATTR_S vpss_grp_attr;
	ISP_IMAGE_ATTR_S isp_image_attr;
    ISP_INPUT_TIMING_S isp_input_timing;
	ISP_GAMMA_TABLE_S pstGammaAttr;
	SDK_ISP_sensor_check();


	memset(&vpss_grp_attr, 0, sizeof(vpss_grp_attr));
	//init sensor
	printf("sensor type:%d\r\n", g_sensor_type);
	switch(g_sensor_type){
		default:
		case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
			APTINA_AR0130_init(ar0130_i2c_read, ar0130_i2c_write); 
			SET_VI_DEV_ATTR_AR0130(vi_dev_attr_720p_30fps);
			break;
		case SDK_ISP_SENSOR_MODEL_OV_OV9712:
			OV9712_init(ov9712_i2c_read, ov9712_i2c_write);
			SET_VI_DEV_ATTR_OV9712(vi_dev_attr_720p_30fps);			
			break;
		case SDK_ISP_SENSOR_MODEL_SOI_H22:
			SOIH22_init(soih22_i2c_read, soih22_i2c_write);
			SET_VI_DEV_ATTR_SOIH22(vi_dev_attr_720p_30fps);			
			break;	
		case SDK_ISP_SENSOR_MODEL_SONY_IMX122:
			SONY_IMX122_init(imx122_i2c_read,imx122_i2c_write);
			SET_VI_DEV_ATTR_IMX122(vi_dev_attr_720p_30fps);
			break;
	}
	
	{
		SOC_CHECK(HI_MPI_VI_SetDevAttr(HI3518A_VIN_DEV, &vi_dev_attr_720p_30fps));
		SOC_CHECK(HI_MPI_VI_EnableDev(HI3518A_VIN_DEV));
	}
	{
	    // ISP isp init
	    
	SOC_CHECK(HI_MPI_ISP_Init());
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&pstGammaAttr)); 
	switch(g_sensor_type){
		default:
		case SDK_ISP_SENSOR_MODEL_APINA_AR0130:
			isp_image_attr.enBayer = BAYER_GRBG;
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SDK_ISP_SENSOR_MODEL_OV_OV9712:
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			isp_image_attr.enBayer = BAYER_BGGR;		
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SDK_ISP_SENSOR_MODEL_SOI_H22:
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			isp_image_attr.enBayer = BAYER_BGGR;	
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SDK_ISP_SENSOR_MODEL_SONY_IMX122:
			vin_chn_attr.stCapRect.u32Width = 1920;
			vin_chn_attr.stCapRect.u32Height = 1080;
			vin_chn_attr.stDestSize.u32Width = 1920;
			vin_chn_attr.stDestSize.u32Height = 1080;
			isp_image_attr.enBayer = BAYER_RGGB;
       		isp_image_attr.u16Width = 1920;
        	isp_image_attr.u16Height = 1080;
			vpss_grp_attr.u32MaxW = 1920;
			vpss_grp_attr.u32MaxH = 1080;			
			isp_input_timing.enWndMode = ISP_WIND_ALL;
			isp_input_timing.u16HorWndStart = 200;
			isp_input_timing.u16HorWndLength = 1920;
			isp_input_timing.u16VerWndStart = 24;
			isp_input_timing.u16VerWndLength = 1080;
			break;
			
	}
	    // ISP set image attributes		
	    isp_image_attr.u16FrameRate = 30;
	    SOC_CHECK(HI_MPI_ISP_SetImageAttr(&isp_image_attr));
	    // ISP set timing
	    SOC_CHECK(HI_MPI_ISP_SetInputTiming(&isp_input_timing));
		
		ret = pthread_create(&isp_tid, NULL, (void*(*)(void*))HI_MPI_ISP_Run, NULL);
		
		assert(0 == ret);
	}
	{
		
		/* step  5: config & start vicap dev */
		vin_chn_attr.stCapRect.s32X = 0;
		vin_chn_attr.stCapRect.s32Y = 0;
		/*vin_chn_attr.stCapRect.u32Width = 1280;
		vin_chn_attr.stCapRect.u32Height = 720;		
		vin_chn_attr.stDestSize.u32Width = 1280;
		vin_chn_attr.stDestSize.u32Height = 720;*/
		vin_chn_attr.enCapSel = VI_CAPSEL_BOTH;
		vin_chn_attr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		vin_chn_attr.bMirror = HI_FALSE;
		vin_chn_attr.bFlip = HI_FALSE;
		vin_chn_attr.bChromaResample = HI_FALSE;
		vin_chn_attr.s32SrcFrameRate = 30;
		vin_chn_attr.s32FrameRate = 30;
	
		SOC_CHECK(HI_MPI_VI_SetChnAttr(HI3518A_VIN_CHN, &vin_chn_attr));

		SDK_ISP_set_mirror(HI3518A_VIN_CHN, false);
		SDK_ISP_set_flip(HI3518A_VIN_CHN, false);
		//SDK_ISP_set_rotate(HI3518A_VIN_CHN, SDK_ISP_ROTATE_0);
		
		SOC_CHECK(HI_MPI_VI_EnableChn(HI3518A_VIN_CHN));
	}
	{
		int vpss_grp = 0; // only use one vpss
		VPSS_GRP_PARAM_S vpss_grp_param;

		memset(&vpss_grp_param, 0, sizeof(vpss_grp_param));


		vpss_grp_attr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		vpss_grp_attr.bDrEn = HI_FALSE;
		vpss_grp_attr.bDbEn = HI_FALSE;
		vpss_grp_attr.bIeEn = HI_TRUE;
		vpss_grp_attr.bNrEn = HI_TRUE;
		vpss_grp_attr.bHistEn = HI_TRUE;
		vpss_grp_attr.enDieMode = VPSS_DIE_MODE_AUTO;

		SOC_CHECK(HI_MPI_VPSS_CreateGrp(vpss_grp, &vpss_grp_attr));
		SOC_CHECK(HI_MPI_VPSS_GetGrpParam(vpss_grp, &vpss_grp_param));
		vpss_grp_param.u32MotionThresh = 0;
		SOC_CHECK(HI_MPI_VPSS_SetGrpParam(vpss_grp, &vpss_grp_param));
		SOC_CHECK(HI_MPI_VPSS_StartGrp(vpss_grp));

		MPP_CHN_S viu_mpp_chn;
		MPP_CHN_S vpss_mpp_chn;

		viu_mpp_chn.enModId = HI_ID_VIU;
		viu_mpp_chn.s32DevId = HI3518A_VIN_DEV;
		viu_mpp_chn.s32ChnId = HI3518A_VIN_CHN;
		vpss_mpp_chn.enModId = HI_ID_VPSS;
		vpss_mpp_chn.s32DevId = 0;
		vpss_mpp_chn.s32ChnId = 0;
		SOC_CHECK(HI_MPI_SYS_Bind(&viu_mpp_chn, &vpss_mpp_chn));
	}

	/*if(g_sensor_type == SDK_ISP_SENSOR_MODEL_OV_OV9712){
		ISP_AE_ATTR_S ae_attr;
		HI_MPI_ISP_GetAEAttr(&ae_attr);
		AEAttr.enFrameEndUpdateMode = ISP_AE_FRAME_END_UPDATE_2;
		HI_MPI_ISP_SetAEAttr(&ae_attr);
	}*/
	SDK_ISP_set_isp_default_value((int)g_sensor_type, 0);
	isp_ircut_gpio_init();
	isp_ircut_switch(0);//default for daylight
	return SDK_SUCCESS;
}

SDK_API_t SDK_ISP_destroy()
{
	return SDK_SUCCESS;
}

