
#include "sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_SOIH22_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_SOIH22_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)
   
static cmos_inttime_t cmos_inttime;
static cmos_gains_t cmos_gains;

static cmos_isp_default_t st_coms_isp_default =
{
	{// color matrix[9] ccm     modified by daniel
        5048,
        {   0x1f5, 0x80be, 0x8037,
            0x8045, 0x195, 0x8050,
            0x800e, 0x8146, 0x255
        },
        3193,
        {   0x1ee, 0x8017, 0x80d6,
            0x8053, 0x1d2, 0x807e,
            0x8023, 0x8176, 0x29a
        },
        2480,
        {   0x21d, 0x8091, 0x808c,
            0x8072, 0x19d, 0x802b,
            0x80b3, 0x8366, 0x519
        }
    }
    ,

	// black level
    {1,1,1,1},  //calibration by tools added by wenyu

    //calibration reference color temperature modified by daniel
    5048,

    //WB gain at 5000, must keep consistent with calibration color temperature
    {0x152, 0x100, 0x100, 0X1ab},

    // WB curve parameters, must keep consistent with reference color temperature.
    {139, -51, -167, 184226, 128, -136368},

	// hist_thresh
	{0xd,0x28,0x60,0x80},
    //{0x10,0x40,0xc0,0xf0},

	0x0,	// iridix_balck
	0x3,	// bggr

	// gain
	0x10,	0x2,

	// iridix
	0x04,	0x08,	0xa0, 	0x4ff,

	0x1, 	// balance_fe
	0x70,	// ae compensation
	0x15, 	// sinter threshold

	0x0,  0,  0  //noise profile=0, use the default noise profile lut, don't need to set nr0 and nr1
};

static cmos_isp_agc_table_t st_isp_agc_table =
{
        //100,200 ,400 ,800 ,1600,3200 ,6400
        //sharpen_alt_d
        //{0xbc,0x6d,0x55,0x60,0x16,0x8e,0xd8,0x01},
		{0xc0,0xb0,0x80,0x30,0x16,0x8e,0xd8,0x01},
        //sharpen_alt_ud
        {0xc0,0xb0,0x70,0x30,0x20,0x3d,0x1e,0x20},

        //snr_thresh modified by daniel
        //{0x1d,0x1e,0x1f,0x4e,0x5a,0x59,0x64,0x59},
        {0x08,0x18,0x1e,0x15,0x38,0x30,0x30,0x30},

        //demosaic_lum_thresh
        {0x40,0x60,0x80,0x80,0x80,0x80,0x80,0x80},

        //demosaic_np_offset
        {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

        //ge_strength
        {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37},

        /* saturation */
        {0x7B,0x7B,0x7B,0x78,0x68,0x48,0x35,0x30}
};
static cmos_isp_demosaic_t st_isp_demosaic =
{
    /*vh_slope*/
    0xff,

    /*aa_slope*/
    0xe4,

    /*va_slope*/
    0xec,

    /*uu_slope*/
    0x9f,

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0x138,

    /*aa_thresh*/
    0xba,

    /*va_thresh*/
    0xda,

    /*uu_thresh*/
    0x148,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};

static cmos_isp_shading_table_t st_isp_shading_table =
{
    /*shading_center_r*/
    0x280, 0x168,

    /*shading_center_g*/
    0x280, 0x168,

    /*shading_center_b*/
    0x280, 0x168,

    /*shading_table_r*/
    {0x1000,0x1018,0x1028,0x103a,0x104c,0x105c,0x1072,0x1089,0x109e,0x10ba,0x10d5,0x10ef,
	0x110b,0x112b,0x114c,0x116d,0x118b,0x11ae,0x11d0,0x11f5,0x1218,0x123e,0x1260,0x1283,
	0x12aa,0x12cf,0x12f7,0x131b,0x1341,0x1369,0x138f,0x13b5,0x13db,0x1401,0x1423,0x1446,
	0x146d,0x148f,0x14b4,0x14d7,0x14fe,0x151e,0x153e,0x155a,0x1579,0x159a,0x15b7,0x15d3,
	0x15f4,0x1612,0x162e,0x164d,0x1663,0x167f,0x169a,0x16b1,0x16cb,0x16e4,0x16fc,0x170f,
	0x1727,0x173e,0x1753,0x176a,0x1783,0x1793,0x17a5,0x17b8,0x17c9,0x17da,0x17ec,0x17fe,
	0x180d,0x181a,0x182a,0x183b,0x184c,0x185a,0x1865,0x1876,0x1883,0x1890,0x189f,0x18a9,
	0x18b5,0x18c3,0x18cf,0x18d8,0x18e2,0x18e8,0x18ec,0x18f5,0x1901,0x190e,0x191f,0x1934,
	0x1946,0x1955,0x1968,0x197e,0x1993,0x19a4,0x19b5,0x19cc,0x19e1,0x19f5,0x1a06,0x1a16,
	0x1a2a,0x1a3d,0x1a4f,0x1a5d,0x1a6e,0x1a84,0x1a96,0x1aa7,0x1abb,0x1ad2,0x1ae5,0x1af9,
	0x1b0e,0x1b27,0x1b40,0x1b59,0x1b68,0x1b72,0x1b91,0x1bbf,0x1bf6},

    /*shading_table_g*/
    {0x1000,0x1018,0x1028,0x103a,0x104c,0x105c,0x1072,0x1089,0x109e,0x10ba,0x10d5,0x10ef,
	0x110b,0x112b,0x114c,0x116d,0x118b,0x11ae,0x11d0,0x11f5,0x1218,0x123e,0x1260,0x1283,
	0x12aa,0x12cf,0x12f7,0x131b,0x1341,0x1369,0x138f,0x13b5,0x13db,0x1401,0x1423,0x1446,
	0x146d,0x148f,0x14b4,0x14d7,0x14fe,0x151e,0x153e,0x155a,0x1579,0x159a,0x15b7,0x15d3,
	0x15f4,0x1612,0x162e,0x164d,0x1663,0x167f,0x169a,0x16b1,0x16cb,0x16e4,0x16fc,0x170f,
	0x1727,0x173e,0x1753,0x176a,0x1783,0x1793,0x17a5,0x17b8,0x17c9,0x17da,0x17ec,0x17fe,
	0x180d,0x181a,0x182a,0x183b,0x184c,0x185a,0x1865,0x1876,0x1883,0x1890,0x189f,0x18a9,
	0x18b5,0x18c3,0x18cf,0x18d8,0x18e2,0x18e8,0x18ec,0x18f5,0x1901,0x190e,0x191f,0x1934,
	0x1946,0x1955,0x1968,0x197e,0x1993,0x19a4,0x19b5,0x19cc,0x19e1,0x19f5,0x1a06,0x1a16,
	0x1a2a,0x1a3d,0x1a4f,0x1a5d,0x1a6e,0x1a84,0x1a96,0x1aa7,0x1abb,0x1ad2,0x1ae5,0x1af9,
	0x1b0e,0x1b27,0x1b40,0x1b59,0x1b68,0x1b72,0x1b91,0x1bbf,0x1bf6},

    /*shading_table_b*/
    {0x1000,0x1018,0x1028,0x103a,0x104c,0x105c,0x1072,0x1089,0x109e,0x10ba,0x10d5,0x10ef,
	0x110b,0x112b,0x114c,0x116d,0x118b,0x11ae,0x11d0,0x11f5,0x1218,0x123e,0x1260,0x1283,
	0x12aa,0x12cf,0x12f7,0x131b,0x1341,0x1369,0x138f,0x13b5,0x13db,0x1401,0x1423,0x1446,
	0x146d,0x148f,0x14b4,0x14d7,0x14fe,0x151e,0x153e,0x155a,0x1579,0x159a,0x15b7,0x15d3,
	0x15f4,0x1612,0x162e,0x164d,0x1663,0x167f,0x169a,0x16b1,0x16cb,0x16e4,0x16fc,0x170f,
	0x1727,0x173e,0x1753,0x176a,0x1783,0x1793,0x17a5,0x17b8,0x17c9,0x17da,0x17ec,0x17fe,
	0x180d,0x181a,0x182a,0x183b,0x184c,0x185a,0x1865,0x1876,0x1883,0x1890,0x189f,0x18a9,
	0x18b5,0x18c3,0x18cf,0x18d8,0x18e2,0x18e8,0x18ec,0x18f5,0x1901,0x190e,0x191f,0x1934,
	0x1946,0x1955,0x1968,0x197e,0x1993,0x19a4,0x19b5,0x19cc,0x19e1,0x19f5,0x1a06,0x1a16,
	0x1a2a,0x1a3d,0x1a4f,0x1a5d,0x1a6e,0x1a84,0x1a96,0x1aa7,0x1abb,0x1ad2,0x1ae5,0x1af9,
	0x1b0e,0x1b27,0x1b40,0x1b59,0x1b68,0x1b72,0x1b91,0x1bbf,0x1bf6},

    /*shading_off_center_r_g_b*/
    0xf0e, 0xf0e, 0xf0e,

    /*shading_table_nobe_number*/
    129
};


static cmos_isp_noise_table_t st_isp_noise_table =
{

//nosie_profile_weight_lut
    {
     0x19,0x20,0x24,0x27,0x29,0x2b,0x2d,0x2e,0x2f,0x31,0x32,0x33,0x34,0x34,0x35,0x36,0x37,\
	0x37,0x38,0x38,0x39,0x3a,0x3a,0x3b,0x3b,0x3b,0x3c,0x3c,0x3d,0x3d,0x3d,0x3e,0x3e,0x3e,\
	0x3f,0x3f,0x3f,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,0x43,0x43,0x43,\
	0x43,0x44,0x44,0x44,0x44,0x44,0x45,0x45,0x45,0x45,0x45,0x46,0x46,0x46,0x46,0x46,0x46,\
	0x47,0x47,0x47,0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,\
	0x49,0x49,0x49,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4b,0x4b,0x4b,0x4b,0x4b,\
	0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4d,0x4d,0x4d,0x4d,\
	0x4d,0x4d,0x4d,0x4d,0x4d,0x4d,0x4e,0x4e,0x4e

    },
    
//demosaic_weight_lut
    {
        0x19,0x20,0x24,0x27,0x29,0x2b,0x2d,0x2e,0x2f,0x31,0x32,0x33,0x34,0x34,0x35,0x36,0x37,\
	0x37,0x38,0x38,0x39,0x3a,0x3a,0x3b,0x3b,0x3b,0x3c,0x3c,0x3d,0x3d,0x3d,0x3e,0x3e,0x3e,\
	0x3f,0x3f,0x3f,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,0x43,0x43,0x43,\
	0x43,0x44,0x44,0x44,0x44,0x44,0x45,0x45,0x45,0x45,0x45,0x46,0x46,0x46,0x46,0x46,0x46,\
	0x47,0x47,0x47,0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,\
	0x49,0x49,0x49,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4b,0x4b,0x4b,0x4b,0x4b,\
	0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4d,0x4d,0x4d,0x4d,\
	0x4d,0x4d,0x4d,0x4d,0x4d,0x4d,0x4e,0x4e,0x4e
    }
    
};

/*
 * This function initialises an instance of cmos_inttime_t.
 */
static cmos_inttime_const_ptr_t soih22_inttime_initialize()
{
#define STD_LINES 767
		cmos_inttime.full_lines_std = STD_LINES;
		cmos_inttime.full_lines_std_30fps = STD_LINES;
		cmos_inttime.full_lines = STD_LINES;
		cmos_inttime.full_lines_limit = 65535;
		cmos_inttime.max_lines_target = STD_LINES;
		cmos_inttime.min_lines_target = 8;
		cmos_inttime.max_lines = STD_LINES;
		cmos_inttime.vblanking_lines = 0;
		cmos_inttime.exposure_ashort = 0;
		cmos_inttime.exposure_shift = 0;
		cmos_inttime.lines_per_500ms = cmos_inttime.full_lines_std_30fps * 30 / 2; // 500ms / 39.17us
		cmos_inttime.flicker_freq = 0;//60*256;//50*256;
	
		return &cmos_inttime;
}

/*
 * This function applies the new integration time to the ISP registers.
 */
static void soih22_inttime_update(cmos_inttime_ptr_t p_inttime)
{
	HI_U32 _curr = p_inttime->exposure_ashort ;
	//refresh the sensor setting every frame to avoid defect pixel error
	
	SENSOR_I2C_WRITE(0x01, _curr&0xFF);
	SENSOR_I2C_WRITE(0x02, (_curr>>8)&0xFF);
}

/*
 * This function applies the new vert blanking porch to the ISP registers.
 */
static void soih22_vblanking_update(cmos_inttime_const_ptr_t p_inttime)
{
    SENSOR_I2C_WRITE(0x22, (p_inttime->full_lines)&0xff);
//    printf("cmos_vblanking_update 0x22 --->%d\n",(p_inttime->full_lines));
	SENSOR_I2C_WRITE(0x23, (p_inttime->full_lines>>8));
//     printf("cmos_vblanking_update 0x23 --->%d\n",(p_inttime->vblanking_lines >> 8));
}

static HI_U16 soih22_vblanking_calculate(
		cmos_inttime_ptr_t p_inttime)
{
	if(p_inttime->exposure_ashort >= p_inttime->full_lines -2)
	{
		p_inttime->exposure_ashort = p_inttime->full_lines -2;
	}
                                 //expecting picture height      valid height of expecting picture  
	p_inttime->vblanking_lines = p_inttime->full_lines - p_inttime->full_lines_std;
	return p_inttime->exposure_ashort;
}

/* Set fps base */
static void soih22_fps_set(
		cmos_inttime_ptr_t p_inttime,
		const HI_U8 fps
		)
{
	switch(fps)
	{
		default://default30fps
		case30:
			p_inttime->full_lines=p_inttime->full_lines_std_30fps;
			p_inttime->lines_per_500ms = cmos_inttime.full_lines_std_30fps * 30 / 2;
//			printf("cmos_fps_set 30fps full_lines==%d\n" ,p_inttime->full_lines);
			SENSOR_I2C_WRITE(0x22, (p_inttime->full_lines)&0xff);
			SENSOR_I2C_WRITE(0x23, (p_inttime->full_lines>>8));
			p_inttime->full_lines_std= p_inttime->full_lines;
			p_inttime->lines_per_500ms=p_inttime->full_lines*30/2;//refresh the deflicker parameter
		break;
		case25:
			p_inttime->full_lines=(30*p_inttime->full_lines_std_30fps)/25;
			p_inttime->lines_per_500ms = cmos_inttime.full_lines_std_30fps * 30 / 2;
   //		printf("cmos_fps_set 25fps full_lines==%d\n" ,p_inttime->full_lines);
			SENSOR_I2C_WRITE(0x22, (p_inttime->full_lines)&0xff);
			SENSOR_I2C_WRITE(0x23, (p_inttime->full_lines>>8));
			p_inttime->full_lines_std= p_inttime->full_lines;
			p_inttime->lines_per_500ms=p_inttime->full_lines*25/2;//refresh the deflicker parameter
		break;
	}

}

/*
 * This function initialises an instance of cmos_gains_t.
 */
static cmos_gains_ptr_t soih22_gains_initialize()
{
    cmos_gains.max_again = 16<<4;
	cmos_gains.max_dgain = 4;

	cmos_gains.again_shift = 4;
	cmos_gains.dgain_shift = 0;
	cmos_gains.dgain_fine_shift = 0;
	cmos_gains.isp_dgain_shift = 4;
	cmos_gains.isp_dgain = 1 << cmos_gains.isp_dgain_shift;
	cmos_gains.max_isp_dgain_target = 1 << cmos_gains.isp_dgain_shift;

	return &cmos_gains;

}

static HI_U32 soih22_get_ISO(cmos_gains_ptr_t p_gains)
{
	HI_U32 _again = p_gains->again == 0 ? 1 : p_gains->again;
	HI_U32 _dgain = p_gains->dgain == 0 ? 1 : p_gains->dgain;

	p_gains->iso =  ((_again * _dgain * 100) >> (p_gains->again_shift + p_gains->dgain_shift));

	return p_gains->iso;

}

/*
 * This function applies the new gains to the ISP registers.
 */
static void soih22_gains_update(cmos_gains_const_ptr_t p_gains)
{
	SENSOR_I2C_WRITE(0x00, p_gains->again_db);//again_db is again_string

	return;
}

static HI_U32 soih22_analog_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift)
{
 	int shift = 0;	
	//prevent overlow of exposure
	while (exposure > (1<<20))
	{
		exposure >>= 1;
		exposure_max >>= 1;
		++shift;
	}
    p_gains->again =1;
	HI_U32  exposure0,exposure1;
    int _i=0;
    unsigned int h[3]={0,0,0};
    exposure0=exposure;
    
        //low 4 bits, step: 1/16 times 
    unsigned int lowbit=0;
    if(exposure>exposure_max)
    {

        for(_i=0;_i<3;_i++)
        {
            exposure1=exposure0>>1;
            if(exposure1<=exposure_max)
                break;
            h[_i]=1;
            exposure0=exposure1;
        }

        for(_i=0;_i<15;_i++)
        {
            exposure1 = exposure0 * 964>> 10;  //   964/1024=1/1.0625    1.0625=1+1/16   add 1.0625 times each time
            if(exposure1<=exposure_max)
                break;
            lowbit ++; 
            exposure0 = exposure1;
        }


    }

 //       printf("lowbit %d \n",lowbit);
        p_gains->again = (exposure << p_gains->again_shift) / exposure0;
        unsigned int regval;
        regval = lowbit | h[0]<<4 | h[1]<<5 | h[2]<<6;
        p_gains->again_db = regval;
        
     return exposure0<<shift;
}

static HI_U32 soih22_digital_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift)
{
	HI_U32 _dgain = (1<<p_gains->dgain_shift);
	int shift = 0;
    int exposure0,exposure1;
    exposure0=exposure;
    int _i;
	//prevent overlow of exposure
	while (exposure > (1<<20))
	{
		exposure >>= 1;
		exposure_max >>= 1;
		++shift;
	}
	
    if(exposure>exposure_max)
    {   
        for(_i=0;_i<1;_i++)
        {
            exposure1=exposure0>>1;
            if(exposure1<exposure_max)
            {
               break;
            }
            else
            {
                _dgain *=2;
                exposure0=exposure1;
            }
            
        }

    }
    p_gains->dgain=_dgain;
    p_gains->dgain_shift=0;
	return exposure0<<shift;

}

static HI_U32 soih22_get_isp_agc_table(cmos_isp_agc_table_ptr_t p_cmos_isp_agc_table)
{
	if (NULL == p_cmos_isp_agc_table)
	{
	    printf("null pointer when get isp agc table value!\n");
	    return -1;
	}
    memcpy(p_cmos_isp_agc_table, &st_isp_agc_table, sizeof(cmos_isp_agc_table_t));
    return 0;
}

static HI_U32 soih22_get_isp_noise_table(cmos_isp_noise_table_ptr_t p_cmos_isp_noise_table)
{
	if (NULL == p_cmos_isp_noise_table)
	{
	    printf("null pointer when get isp noise table value!\n");
	    return -1;
	}
    memcpy(p_cmos_isp_noise_table, &st_isp_noise_table, sizeof(cmos_isp_noise_table_t));
    return 0;
}

static HI_U32 soih22_get_isp_demosaic(cmos_isp_demosaic_ptr_t p_cmos_isp_demosaic)
{
   if (NULL == p_cmos_isp_demosaic)
   {
	    printf("null pointer when get isp demosaic value!\n");
	    return -1;
   }
   memcpy(p_cmos_isp_demosaic, &st_isp_demosaic,sizeof(cmos_isp_demosaic_t));
   return 0;

}

static HI_U32 soih22_get_isp_shading_table(cmos_isp_shading_table_ptr_t p_cmos_isp_shading_table)
{
	if (NULL == p_cmos_isp_shading_table)
	{
	    printf("null pointer when get isp shading table value!\n");
	    return -1;
	}
    memcpy(p_cmos_isp_shading_table, &st_isp_shading_table, sizeof(cmos_isp_shading_table_t));
    return 0;
}


static void soih22_setup_sensor(int isp_mode)
{
	if(0 == isp_mode) /* ISP 'normal' isp_mode */
	{
		int lines0=(767);//normal full lines is 767 lines ,30 fps
//		printf("set up sensor ispmode ==1--->lines =%d \n",lines0);
		SENSOR_I2C_WRITE(0x22, (lines0)&0xff);
		SENSOR_I2C_WRITE(0x23, (lines0>>8));
	}
	else if(1 == isp_mode) /* ISP pixel calibration isp_mode */
	{
		SENSOR_I2C_WRITE(0x00, 0x00);//again=1;
		SENSOR_I2C_WRITE(0x0d,0x01);//dgain=1;
		
		int lines1=((30*767)/5);
  //	  printf("set up sensor ispmode ==1--->lines =%d \n",lines1);
		SENSOR_I2C_WRITE(0x22, (lines1)&0xff);
		SENSOR_I2C_WRITE(0x23, (lines1>>8));
	}
}

static HI_U32 soih22_gains_lin_to_db_convert(HI_U32 data, HI_U32 shift_in)
{
    #define PRECISION 8
    HI_U32 _res = 0;
    if(0 == data)
        return _res;

    data = data << PRECISION; // to ensure precision.
    for(;;)
    {
        /* Note to avoid endless loop here. */
        data = (data * 913) >> 10;
        // data = (data*913 + (1<<9)) >> 10; // endless loop when shift_in is 0. */
        if(data <= ((1<<shift_in) << PRECISION))
        {
            break;
        }
        ++_res;
    }
    return _res;
}

static HI_U8 soih22_get_analog_gain(cmos_gains_ptr_t p_gains)
{
    return soih22_gains_lin_to_db_convert(p_gains->again, p_gains->again_shift);
}

static HI_U8 soih22_get_digital_gain(cmos_gains_ptr_t p_gains)
{
    return p_gains->dgain;
}

static HI_U8 soih22_get_digital_fine_gain(cmos_gains_ptr_t p_gains)
{
    return 0;
}

static HI_U32 soih22_get_isp_default(cmos_isp_default_ptr_t p_coms_isp_default)
{
	if (NULL == p_coms_isp_default)
	{
	    printf("null pointer when get isp default value!\n");
	    return -1;
	}
    memcpy(p_coms_isp_default, &st_coms_isp_default, sizeof(cmos_isp_default_t));
    return 0;
}



void SOIH22_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write)
{
	SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;
	
	SENSOR_I2C_WRITE(0x0e, 0x1D);
	SENSOR_I2C_WRITE(0x0f, 0x0B);
	SENSOR_I2C_WRITE(0x10, 0x26);
	SENSOR_I2C_WRITE(0x11, 0x80);
	SENSOR_I2C_WRITE(0x1B, 0x4F);
	SENSOR_I2C_WRITE(0x1D, 0xFF);
	
	SENSOR_I2C_WRITE(0x1E, 0x9F);
	SENSOR_I2C_WRITE(0x20, 0x72);
	SENSOR_I2C_WRITE(0x21, 0x06);
	SENSOR_I2C_WRITE(0x22, 0xFF);
	SENSOR_I2C_WRITE(0x23, 0x02);
	SENSOR_I2C_WRITE(0x24, 0x00);
	SENSOR_I2C_WRITE(0x25, 0xE0);
	SENSOR_I2C_WRITE(0x26, 0x25);
	SENSOR_I2C_WRITE(0x27, 0xE9);
	SENSOR_I2C_WRITE(0x28, 0x0D);
	SENSOR_I2C_WRITE(0x29, 0x00);
	SENSOR_I2C_WRITE(0x2C, 0x00);
	SENSOR_I2C_WRITE(0x2D, 0x08);
	SENSOR_I2C_WRITE(0x2E, 0xC4);
	SENSOR_I2C_WRITE(0x2F, 0x20);
	SENSOR_I2C_WRITE(0x6C, 0x90);
	SENSOR_I2C_WRITE(0x2A, 0xD4);
	SENSOR_I2C_WRITE(0x30, 0x90);
	SENSOR_I2C_WRITE(0x31, 0x10);
	SENSOR_I2C_WRITE(0x32, 0x10);
	SENSOR_I2C_WRITE(0x33, 0x10);
	SENSOR_I2C_WRITE(0x34, 0x32);
	SENSOR_I2C_WRITE(0x14, 0x80);
	SENSOR_I2C_WRITE(0x18, 0xD5);
	SENSOR_I2C_WRITE(0x19, 0x10);
	
	SENSOR_I2C_WRITE(0x0d, 0x00);
	SENSOR_I2C_WRITE(0x1f, 0x00);
	
	SENSOR_I2C_WRITE(0x13, 0x87);
	SENSOR_I2C_WRITE(0x4A, 0x03);
	SENSOR_I2C_WRITE(0x49, 0x06); 
	

	memset(&sensor_exp_func, 0, sizeof(sensor_exp_func));	
	sensor_exp_func.pfn_cmos_inttime_initialize = soih22_inttime_initialize;
    sensor_exp_func.pfn_cmos_inttime_update = soih22_inttime_update;

    sensor_exp_func.pfn_cmos_gains_initialize = soih22_gains_initialize;
    sensor_exp_func.pfn_cmos_gains_update = soih22_gains_update;
    sensor_exp_func.pfn_cmos_gains_update2 = NULL;
   	sensor_exp_func.pfn_analog_gain_from_exposure_calculate = soih22_analog_gain_from_exposure_calculate;
    sensor_exp_func.pfn_digital_gain_from_exposure_calculate = soih22_digital_gain_from_exposure_calculate;

    sensor_exp_func.pfn_cmos_fps_set = soih22_fps_set;
    sensor_exp_func.pfn_vblanking_calculate = soih22_vblanking_calculate;
    sensor_exp_func.pfn_cmos_vblanking_front_update = soih22_vblanking_update;

    sensor_exp_func.pfn_setup_sensor = soih22_setup_sensor;

	sensor_exp_func.pfn_cmos_get_analog_gain = soih22_get_analog_gain;
	sensor_exp_func.pfn_cmos_get_digital_gain = soih22_get_digital_gain;
	sensor_exp_func.pfn_cmos_get_digital_fine_gain = NULL;
    sensor_exp_func.pfn_cmos_get_iso = soih22_get_ISO;

	sensor_exp_func.pfn_cmos_get_isp_default = soih22_get_isp_default;
	sensor_exp_func.pfn_cmos_get_isp_special_alg = NULL;
	sensor_exp_func.pfn_cmos_get_isp_agc_table = soih22_get_isp_agc_table,
	sensor_exp_func.pfn_cmos_get_isp_noise_table = soih22_get_isp_noise_table;
	sensor_exp_func.pfn_cmos_get_isp_demosaic = soih22_get_isp_demosaic;
	sensor_exp_func.pfn_cmos_get_isp_shading_table = soih22_get_isp_shading_table;
	SOC_CHECK(HI_MPI_ISP_SensorRegCallBack(&sensor_exp_func));

	printf("SOIH22 sensor 720P30fps init success!\n");
		
}


