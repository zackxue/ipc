
#include "sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_SONY_IMX122_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_SONY_IMX122_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#define EXPOSURE_ADDR (0x208) //2:chip_id, 0C: reg addr.
#define PGC_ADDR (0x21E)
#define VMAX_ADDR (0x205)

static cmos_inttime_t cmos_inttime;
static cmos_gains_t cmos_gains;

static cmos_isp_default_t st_coms_isp_default =
{
    // color correction matrix
    {
        5000,
    	{	0x1b7,  0x8079, 0x803d,
    		0x806d, 0x01f2, 0x8084,
    		0x800a, 0x80b9, 0x01c4
    	},
    	3200,
        {
            0x01e7, 0x80cd, 0x801a,
            0x808f, 0x01d3, 0x8044,
            0x001b, 0x813b, 0x021f
        },
        2600,
        {
            0x020a, 0x80ed, 0x801d,
            0x806e, 0x0196, 0x8028,
            0x0015, 0x820f, 0x02f9
        }
    },

	// black level for R, Gr, Gb, B channels
	{0xf0,0xf0,0xf0,0xf0},

    // calibration reference color temperature
    5000,

    //WB gain at 5000K, must keep consistent with calibration color temperature
	{0x1c5, 0x100, 0x100, 0x1ec},

    // WB curve parameters, must keep consistent with reference color temperature.
	{22, 141, -84, 186260, 0x80, -134565},

	// hist_thresh
	{0xd,0x28,0x60,0x80},

	0x00,	// iridix_balck
	0x0,	// rggb

	// gain
	0x10,	0x8,

	// iridix space, intensity, slope_max, white level
	0x02,	0x08,	0x80, 	0x8ff,

	0x1, 	// balance_fe
	0x80,	// ae compensation
	0x20, 	// sinter threshold

	0x1,        //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
	0,
	1528
};

static cmos_isp_agc_table_t st_isp_agc_table =
{
    //sharpen_alt_d
    {0x88,0x85,0x80,0x7b,0x78,0x72,0x70,0x60},

    //sharpen_alt_ud
    {0xc8,0xc0,0xb8,0xb0,0xa8,0xa0,0x72,0x4b},

    //snr_thresh
    {0x06,0x8,0xb,0x16,0x22,0x28,0x32,0x54},

    //demosaic_lum_thresh
    {0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80},

    //demosaic_np_offset
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

    //ge_strength
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37},

    /* saturation */
    {0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}
};


static cmos_isp_noise_table_t st_isp_noise_table =
{
  //nosie_profile_weight_lut

    {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x0c,0x11,0x14,0x17,0x19,0x1b,0x1c,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x24,0x25,0x26,0x26,0x27,0x28,0x28,0x29,0x29,0x2a,0x2a,0x2a,
    0x2b,0x2b,0x2c,0x2c,0x2c,0x2d,0x2d,0x2d,0x2e,0x2e,0x2e,0x2f,0x2f,0x2f,0x30,0x30,0x30,
    0x30,0x31,0x31,0x31,0x31,0x32,0x32,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x34,0x34,0x34,
    0x34,0x34,0x34,0x35,0x35,0x35,0x35,0x35,0x35,0x36,0x36,0x36,0x36,0x36,0x36,0x37,0x37,
    0x37,0x37,0x37,0x37,0x37,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x39,0x39,0x39,0x39,0x39,
    0x39,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3b,
    0x3b,0x3b,0x3b,0x3b,0x3b,0x3c,0x3c,0x3c,0x3c
    },
  
    {
    0x04,0x0c,0x11,0x14,0x17,0x19,0x1b,0x1c,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x24,0x25,0x26,0x26,0x27,0x28,0x28,0x29,0x29,0x2a,0x2a,0x2a,
    0x2b,0x2b,0x2c,0x2c,0x2c,0x2d,0x2d,0x2d,0x2e,0x2e,0x2e,0x2f,0x2f,0x2f,0x30,0x30,0x30,
    0x30,0x31,0x31,0x31,0x31,0x32,0x32,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x34,0x34,0x34,
    0x34,0x34,0x34,0x35,0x35,0x35,0x35,0x35,0x35,0x36,0x36,0x36,0x36,0x36,0x36,0x37,0x37,
    0x37,0x37,0x37,0x37,0x37,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x39,0x39,0x39,0x39,0x39,
    0x39,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3b,
    0x3b,0x3b,0x3b,0x3b,0x3b,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c
    },
    
};

static cmos_isp_demosaic_t st_isp_demosaic =
{
    /*vh_slope*/
    0xf5,

    /*aa_slope*/
    0x98,

    /*va_slope*/
    0xe6,

    /*uu_slope*/
    0x90,

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0x32,

    /*aa_thresh*/
    0x5b,

    /*va_thresh*/
    0x52,

    /*uu_thresh*/
    0x40,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3,
};


/*
 * This function initialises an instance of cmos_inttime_t.
 */
static cmos_inttime_const_ptr_t imx122_inttime_initialize()
{
	cmos_inttime.full_lines_std = 1125;
	cmos_inttime.full_lines_std_30fps = 1125;
	cmos_inttime.full_lines = 1125;
	cmos_inttime.full_lines_limit = 65535;
	cmos_inttime.max_lines_target = 1123;
	cmos_inttime.min_lines_target = 2;
	cmos_inttime.vblanking_lines = 1125;

	cmos_inttime.exposure_ashort = 0;

	cmos_inttime.lines_per_500ms = 16874; // 500ms / 29.63us = 16874
	cmos_inttime.flicker_freq = 0;//60*256;//50*256;

	return &cmos_inttime;
}

/*
 * This function applies the new integration time to the ISP registers.
 */
static void imx122_inttime_update(cmos_inttime_ptr_t p_inttime) 
{
	HI_U16 exp_time;
    exp_time = p_inttime->full_lines - p_inttime->exposure_ashort;

    SENSOR_I2C_WRITE(EXPOSURE_ADDR, exp_time & 0xFF);
    SENSOR_I2C_WRITE(EXPOSURE_ADDR + 1, (exp_time & 0xFF00) >> 8);
}

/*
 * This function applies the new vert blanking porch to the ISP registers.
 */
static void imx122_vblanking_update(cmos_inttime_const_ptr_t p_inttime)
{
	   HI_U16 vmax = p_inttime->full_lines;
	//		   printf("vmax=%d",vmax);
	   SENSOR_I2C_WRITE(VMAX_ADDR, (vmax&0x00ff));
	   SENSOR_I2C_WRITE(VMAX_ADDR+1, ((vmax&0xff00) >> 8));
	   
	   return;
}

static __inline HI_U16 imx122_vblanking_calculate(
		cmos_inttime_ptr_t p_inttime)
{
	if (p_inttime->exposure_ashort >= p_inttime->full_lines - 3)
		{
			p_inttime->exposure_ashort = p_inttime->full_lines - 3;
		}
	
		p_inttime->vblanking_lines = p_inttime->full_lines - p_inttime->full_lines_std;
		
	//	  printf("vblanking_lines=%d",p_inttime->vblanking_lines);
	
		return p_inttime->exposure_ashort;
}

/* Set fps base */
static void imx122_fps_set(
		cmos_inttime_ptr_t p_inttime,
		const HI_U8 fps
		)
{
	switch(fps)
	{
		case 30:
			// Change the frame rate via changing the vertical blanking
			p_inttime->full_lines_std = 1125;
			SENSOR_I2C_WRITE(VMAX_ADDR, 0x65);
			SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x04);
			p_inttime->lines_per_500ms = 1125 * 30 / 2;
		break;
		
		case 25:
			// Change the frame rate via changing the vertical blanking
			p_inttime->full_lines_std = 1350;
			SENSOR_I2C_WRITE(VMAX_ADDR, 0x46);
			SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x05);
			p_inttime->lines_per_500ms = 1350 * 25 / 2;
		break;
		
		default:
		break;
	}
}

/*
 * This function initialises an instance of cmos_gains_t.
 */
static cmos_gains_ptr_t imx122_gains_initialize()
{
	cmos_gains.again_shift = 4;
	cmos_gains.dgain_shift = 4;
	cmos_gains.dgain_fine_shift = 0;
    
	cmos_gains.max_again = 16 << cmos_gains.again_shift;  //linear
	cmos_gains.max_dgain = 8 << cmos_gains.dgain_shift; //linear
	
    cmos_gains.isp_dgain_shift = 4;
    cmos_gains.isp_dgain = 1 << cmos_gains.isp_dgain_shift;
    cmos_gains.max_isp_dgain_target = 1 << cmos_gains.isp_dgain_shift;

	return &cmos_gains;

}

static HI_U16 imx122_digital_gain_lut_get_value(HI_U8 index)
{
    static HI_U16 gain_lut[] = 
    {
        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,0x8A,0x8B,0x8C
    };
	return gain_lut[index];
}

/*
 * This function applies the new gains to the ISP registers.
 */
static void imx122_gains_update(cmos_gains_const_ptr_t p_gains)
{
	HI_U16 data16;
	HI_U16 lut_val;

    if((p_gains->again_db + p_gains->dgain_db)< 0x8C)
	{  
	  SENSOR_I2C_WRITE(PGC_ADDR, (p_gains->again_db + p_gains->dgain_db));
    }
    else
    {
      SENSOR_I2C_WRITE(PGC_ADDR, 0x8C);
    }

}

static HI_U32 imx122_gains_lin_to_db_convert(HI_U32 data, HI_U32 shift_in)
{
    int const PRECISION = 8;
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

static HI_U32 imx122_analog_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift)
{
	int _i;
	HI_U32 _again = 0;
	HI_U32 exposure0, exposure1;
	int shft = 0;
	// normalize

	while (exposure > (1<<22))
	{
		exposure >>= 1;
		exposure_max >>= 1;
		++shft;
	}

    exposure0 = exposure;
       /* again unit: 0.3db */
	for(_i = 1; _i <= 80; _i++)
	{
		exposure1 = (exposure0*989) >> 10;
		if(exposure1 <= exposure_max)
			break;
		++_again;
		exposure0 = exposure1;
	}
	p_gains->again = (exposure << p_gains->again_shift) / exposure0; 
	p_gains->again_db = _again;
	return exposure0 << shft;

}

static HI_U32 imx122_digital_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift
		)
{
	int _i;
	HI_U32 _dgain = 0;
	HI_U32 exposure0, exposure1;
	int shft = 0;
	// normalize
	while (exposure > (1<<20)) /* analog use (1<<22) for analog exposure is bigger. */
	{
		exposure >>= 1;
		exposure_max >>= 1;
		++shft;
	}

	exposure0 = exposure;

	/* unit: 0.3db */
	for(_i = 1; _i <= 0x3C; _i++)
	{
		exposure1 = (exposure0*989) >> 10;
		if(exposure1 <= exposure_max)
			break;
		++_dgain;
		exposure0 = exposure1;
	}
	p_gains->dgain = (exposure << p_gains->dgain_shift) / exposure0; 
       p_gains->dgain_db = _dgain; 	
	return exposure0 << shft;

}

static void imx122_sensor_update(
	cmos_gains_const_ptr_t p_gains,
	cmos_inttime_ptr_t p_inttime,
	HI_U8 frame
    )
{
	if(frame == 0)
	{
		imx122_inttime_update(p_inttime);
	}
	if(frame == 1)
	{
		imx122_gains_update(p_gains);
	}    
}

static HI_U32 imx122_get_ISO(cmos_gains_ptr_t p_gains)
{
	HI_U32 _again;
	HI_U32 _dgain;

    _again = p_gains->again;
    _dgain = p_gains->dgain;
	p_gains->iso =  (((_again) * (_dgain) * 100) >> (4 + 4));

	return p_gains->iso;
}

static HI_U8 imx122_get_analog_gain(cmos_gains_ptr_t cmos_gains)
{
    return (cmos_gains->again_db *  3 / 10); 
}


static HI_U8 imx122_get_digital_gain(cmos_gains_ptr_t cmos_gains)
{
    return  (cmos_gains->dgain_db *  3 / 10); 
}

static HI_U8 imx122_get_digital_fine_gain(cmos_gains_ptr_t cmos_gains)
{
    return cmos_gains->dgain_fine;
}


static HI_U32 imx122_get_isp_default(cmos_isp_default_ptr_t p_coms_isp_default)
{
	if (NULL == p_coms_isp_default)
	{
		printf("null pointer when get isp default value!\n");
		return -1;
	}
	memcpy(p_coms_isp_default, &st_coms_isp_default, sizeof(cmos_isp_default_t));
	return 0;
}

HI_U32 imx122_get_isp_speical_alg(void)
{
    return isp_special_alg_awb;
}

static HI_U32 imx122_get_isp_agc_table(cmos_isp_agc_table_ptr_t p_cmos_isp_agc_table)
{
	if (NULL == p_cmos_isp_agc_table)
	{
		printf("null pointer when get isp agc table value!\n");
		return -1;
	}
	memcpy(p_cmos_isp_agc_table, &st_isp_agc_table, sizeof(cmos_isp_agc_table_t));
	return 0;
}

static HI_U32 imx122_get_isp_noise_table(cmos_isp_noise_table_ptr_t p_cmos_isp_noise_table)
{
	if (NULL == p_cmos_isp_noise_table)
	{
		printf("null pointer when get isp noise table value!\n");
		return -1;
	}
	memcpy(p_cmos_isp_noise_table, &st_isp_noise_table, sizeof(cmos_isp_noise_table_t));
	return 0;
}

static HI_U32 imx122_get_isp_demosaic(cmos_isp_demosaic_ptr_t p_cmos_isp_demosaic)
{
   if (NULL == p_cmos_isp_demosaic)
   {
		printf("null pointer when get isp demosaic value!\n");
		return -1;
   }
   memcpy(p_cmos_isp_demosaic, &st_isp_demosaic,sizeof(cmos_isp_demosaic_t));
   return 0;

}
static void imx122_setup_sensor(int isp_mode)
{
	if(0 == isp_mode) /* setup for ISP 'normal mode' */
	{
        SENSOR_I2C_WRITE(VMAX_ADDR, 0x65);
        SENSOR_I2C_WRITE(VMAX_ADDR + 1, 0x04);
	}
	else if(1 == isp_mode) /* setup for ISP pixel calibration mode */
	{
        //TODO: finish this.
        /* Sensor must be programmed for slow frame rate (5 fps and below)*/
        /* change frame rate to 3 fps by setting 1 frame length = 1125 * (30/3) */
        SENSOR_I2C_WRITE(VMAX_ADDR, 0xF2);
        SENSOR_I2C_WRITE(VMAX_ADDR + 1, 0x2B);

        /* Analog and Digital gains both must be programmed for their minimum values */
		SENSOR_I2C_WRITE(PGC_ADDR, 0x00);
       // SENSOR_I2C_WRITE(APGC_ADDR + 1, 0x00);
	//	SENSOR_I2C_WRITE(DPGC_ADDR, 0x00);
	}
}


void SONY_IMX122_init(SENSOR_SONY_IMX122_DO_I2CRD do_i2c_read, SENSOR_SONY_IMX122_DO_I2CWR do_i2c_write)
{
	SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;
		
		SENSOR_I2C_WRITE(0x200, 0x31);
		SENSOR_I2C_WRITE(0x211, 0x00);
		SENSOR_I2C_WRITE(0x22D, 0x40);
		SENSOR_I2C_WRITE(0x202, 0x0F);
		SENSOR_I2C_WRITE(0x216, 0x3C);
		SENSOR_I2C_WRITE(0x217, 0x00);
		SENSOR_I2C_WRITE(0x214, 0x00);
		SENSOR_I2C_WRITE(0x215, 0x00);
		SENSOR_I2C_WRITE(0x218, 0xC0);
		SENSOR_I2C_WRITE(0x219, 0x07);
		SENSOR_I2C_WRITE(0x2CE, 0x16);
		SENSOR_I2C_WRITE(0x2CF, 0x82);
		SENSOR_I2C_WRITE(0x2D0, 0x00);
		SENSOR_I2C_WRITE(0x29A, 0x26);
		SENSOR_I2C_WRITE(0x29B, 0x02);
		SENSOR_I2C_WRITE(0x212, 0x82);
		SENSOR_I2C_WRITE(0x20F, 0x00);
		SENSOR_I2C_WRITE(0x20D, 0x00);
		SENSOR_I2C_WRITE(0x208, 0x00);
		SENSOR_I2C_WRITE(0x209, 0x00);
		SENSOR_I2C_WRITE(0x21E, 0x00);
		SENSOR_I2C_WRITE(0x220, 0xF0);
		SENSOR_I2C_WRITE(0x221, 0x00);
		SENSOR_I2C_WRITE(0x222, 0x40);
		SENSOR_I2C_WRITE(0x205, 0x65);
		SENSOR_I2C_WRITE(0x206, 0x04);
		SENSOR_I2C_WRITE(0x203, 0x4C);
		SENSOR_I2C_WRITE(0x204, 0x04);
		SENSOR_I2C_WRITE(0x23B, 0xE0);
	
			// master mode start
		SENSOR_I2C_WRITE(0x22C, 0x00);
	
	
		SENSOR_I2C_WRITE(0x201, 0x00);
	
	
	
		SENSOR_I2C_WRITE(0x207, 0x00);
	
		SENSOR_I2C_WRITE(0x20A, 0x00);
		SENSOR_I2C_WRITE(0x20B, 0x00);
		SENSOR_I2C_WRITE(0x20C, 0x00);
	
		SENSOR_I2C_WRITE(0x20E, 0x00);
	
		SENSOR_I2C_WRITE(0x210, 0x00);
		
	
		SENSOR_I2C_WRITE(0x213, 0x40);
	
	
	
		SENSOR_I2C_WRITE(0x21A, 0xC9);
		SENSOR_I2C_WRITE(0x21B, 0x04);
		SENSOR_I2C_WRITE(0x21C, 0x50);
		SENSOR_I2C_WRITE(0x21D, 0x00);
	
		SENSOR_I2C_WRITE(0x21F, 0x31);
	
	
		SENSOR_I2C_WRITE(0x223, 0x08);
		SENSOR_I2C_WRITE(0x224, 0x30);
		SENSOR_I2C_WRITE(0x225, 0x00);
		SENSOR_I2C_WRITE(0x226, 0x80);
		SENSOR_I2C_WRITE(0x227, 0x20);
		SENSOR_I2C_WRITE(0x228, 0x34);
		SENSOR_I2C_WRITE(0x229, 0x63);
		SENSOR_I2C_WRITE(0x22A, 0x00);
		SENSOR_I2C_WRITE(0x22B, 0x00);
		
	
		SENSOR_I2C_WRITE(0x22E, 0x00);
		SENSOR_I2C_WRITE(0x22F, 0x02);
	
	
		
		SENSOR_I2C_WRITE(0x230, 0x30);
		SENSOR_I2C_WRITE(0x231, 0x20);
		SENSOR_I2C_WRITE(0x232, 0x00);
		SENSOR_I2C_WRITE(0x233, 0x14);
		SENSOR_I2C_WRITE(0x234, 0x20);
		SENSOR_I2C_WRITE(0x235, 0x60);
		SENSOR_I2C_WRITE(0x236, 0x00);
		SENSOR_I2C_WRITE(0x237, 0x23);
		SENSOR_I2C_WRITE(0x238, 0x01);
		SENSOR_I2C_WRITE(0x239, 0x00);
		SENSOR_I2C_WRITE(0x23A, 0xA8);
		SENSOR_I2C_WRITE(0x23B, 0xE0);
		SENSOR_I2C_WRITE(0x23C, 0x06);
		SENSOR_I2C_WRITE(0x23D, 0x00);
		SENSOR_I2C_WRITE(0x23E, 0x10);
		SENSOR_I2C_WRITE(0x23F, 0x00);
		SENSOR_I2C_WRITE(0x240, 0x42);
		SENSOR_I2C_WRITE(0x241, 0x23);
		SENSOR_I2C_WRITE(0x242, 0x3C);
		SENSOR_I2C_WRITE(0x243, 0x01);
		SENSOR_I2C_WRITE(0x244, 0x00);
		SENSOR_I2C_WRITE(0x245, 0x00);
		SENSOR_I2C_WRITE(0x246, 0x00);
		SENSOR_I2C_WRITE(0x247, 0x00);
		SENSOR_I2C_WRITE(0x248, 0x00);
		SENSOR_I2C_WRITE(0x249, 0x00);
		SENSOR_I2C_WRITE(0x24A, 0x00);
		SENSOR_I2C_WRITE(0x24B, 0x00);
		SENSOR_I2C_WRITE(0x24C, 0x01);
		SENSOR_I2C_WRITE(0x24D, 0x00);
		SENSOR_I2C_WRITE(0x24E, 0x01);
		SENSOR_I2C_WRITE(0x24F, 0x47);
		SENSOR_I2C_WRITE(0x250, 0x10);
		SENSOR_I2C_WRITE(0x251, 0x18);
		SENSOR_I2C_WRITE(0x252, 0x12);
		SENSOR_I2C_WRITE(0x253, 0x00);
		SENSOR_I2C_WRITE(0x254, 0x00);
		SENSOR_I2C_WRITE(0x255, 0x00);
		SENSOR_I2C_WRITE(0x256, 0x00);
		SENSOR_I2C_WRITE(0x257, 0x00);
		SENSOR_I2C_WRITE(0x258, 0xE0);
		SENSOR_I2C_WRITE(0x259, 0x01);
		SENSOR_I2C_WRITE(0x25A, 0xE0);
		SENSOR_I2C_WRITE(0x25B, 0x01);
		SENSOR_I2C_WRITE(0x25C, 0x00);
		SENSOR_I2C_WRITE(0x25D, 0x00);
		SENSOR_I2C_WRITE(0x25E, 0x00);
		SENSOR_I2C_WRITE(0x25F, 0x00);
		SENSOR_I2C_WRITE(0x260, 0x00);
		SENSOR_I2C_WRITE(0x261, 0x00);
		SENSOR_I2C_WRITE(0x262, 0x76);
		SENSOR_I2C_WRITE(0x263, 0x00);
		SENSOR_I2C_WRITE(0x264, 0x01);
		SENSOR_I2C_WRITE(0x265, 0x00);
		SENSOR_I2C_WRITE(0x266, 0x00);
		SENSOR_I2C_WRITE(0x267, 0x00);
		SENSOR_I2C_WRITE(0x268, 0x00);
		SENSOR_I2C_WRITE(0x269, 0x00);
		SENSOR_I2C_WRITE(0x26A, 0x00);
		SENSOR_I2C_WRITE(0x26B, 0x00);
		SENSOR_I2C_WRITE(0x26C, 0x00);
		SENSOR_I2C_WRITE(0x26D, 0x00);
		SENSOR_I2C_WRITE(0x26E, 0x00);
		SENSOR_I2C_WRITE(0x26F, 0x00);
		SENSOR_I2C_WRITE(0x270, 0x00);
		SENSOR_I2C_WRITE(0x271, 0x00);
		SENSOR_I2C_WRITE(0x272, 0x00);
		SENSOR_I2C_WRITE(0x273, 0x01);
		SENSOR_I2C_WRITE(0x274, 0x06);
		SENSOR_I2C_WRITE(0x275, 0x07);
		SENSOR_I2C_WRITE(0x276, 0x80);
		SENSOR_I2C_WRITE(0x277, 0x00);
		SENSOR_I2C_WRITE(0x278, 0x40);
		SENSOR_I2C_WRITE(0x279, 0x08);
		SENSOR_I2C_WRITE(0x27A, 0x00);
		SENSOR_I2C_WRITE(0x27B, 0x00);
		SENSOR_I2C_WRITE(0x27C, 0x10);
		SENSOR_I2C_WRITE(0x27D, 0x00);
		SENSOR_I2C_WRITE(0x27E, 0x00);
		SENSOR_I2C_WRITE(0x27F, 0x00);
		SENSOR_I2C_WRITE(0x280, 0x06);
		SENSOR_I2C_WRITE(0x281, 0x19);
		SENSOR_I2C_WRITE(0x282, 0x00);
		SENSOR_I2C_WRITE(0x283, 0x64);
		SENSOR_I2C_WRITE(0x284, 0x00);
		SENSOR_I2C_WRITE(0x285, 0x01);
		SENSOR_I2C_WRITE(0x286, 0x00);
		SENSOR_I2C_WRITE(0x287, 0x00);
		SENSOR_I2C_WRITE(0x288, 0x00);
		SENSOR_I2C_WRITE(0x289, 0x00);
		SENSOR_I2C_WRITE(0x28A, 0x00);
		SENSOR_I2C_WRITE(0x28B, 0x00);
		SENSOR_I2C_WRITE(0x28C, 0x00);
		SENSOR_I2C_WRITE(0x28D, 0x00);
		SENSOR_I2C_WRITE(0x28E, 0x00);
		SENSOR_I2C_WRITE(0x28F, 0x00);
		SENSOR_I2C_WRITE(0x290, 0x00);
		SENSOR_I2C_WRITE(0x291, 0x00);
		SENSOR_I2C_WRITE(0x292, 0x01);
		SENSOR_I2C_WRITE(0x293, 0x01);
		SENSOR_I2C_WRITE(0x294, 0x00);
		SENSOR_I2C_WRITE(0x295, 0xFF);
		SENSOR_I2C_WRITE(0x296, 0x0F);
		SENSOR_I2C_WRITE(0x297, 0x00);
		SENSOR_I2C_WRITE(0x298, 0x26);
		SENSOR_I2C_WRITE(0x299, 0x02);
	
		SENSOR_I2C_WRITE(0x29C, 0x9C);
		SENSOR_I2C_WRITE(0x29D, 0x01);
		SENSOR_I2C_WRITE(0x29E, 0x39);
		SENSOR_I2C_WRITE(0x29F, 0x03);
		SENSOR_I2C_WRITE(0x2A0, 0x01);
		SENSOR_I2C_WRITE(0x2A1, 0x05);
		SENSOR_I2C_WRITE(0x2A2, 0xD0);
		SENSOR_I2C_WRITE(0x2A3, 0x07);
		SENSOR_I2C_WRITE(0x2A4, 0x00);
		SENSOR_I2C_WRITE(0x2A5, 0x02);
		SENSOR_I2C_WRITE(0x2A6, 0x0B);
		SENSOR_I2C_WRITE(0x2A7, 0x0F);
		SENSOR_I2C_WRITE(0x2A8, 0x24);
		SENSOR_I2C_WRITE(0x2A9, 0x00);
		SENSOR_I2C_WRITE(0x2AA, 0x28);
		SENSOR_I2C_WRITE(0x2AB, 0x00);
		SENSOR_I2C_WRITE(0x2AC, 0xE8);
		SENSOR_I2C_WRITE(0x2AD, 0x04);
		SENSOR_I2C_WRITE(0x2AE, 0xEC);
		SENSOR_I2C_WRITE(0x2AF, 0x04);
		SENSOR_I2C_WRITE(0x2B0, 0x00);
		SENSOR_I2C_WRITE(0x2B1, 0x00);
		SENSOR_I2C_WRITE(0x2B2, 0x03);
		SENSOR_I2C_WRITE(0x2B3, 0x05);
		SENSOR_I2C_WRITE(0x2B4, 0x00);
		SENSOR_I2C_WRITE(0x2B5, 0x0F);
		SENSOR_I2C_WRITE(0x2B6, 0x10);
		SENSOR_I2C_WRITE(0x2B7, 0x00);
		SENSOR_I2C_WRITE(0x2B8, 0x28);
		SENSOR_I2C_WRITE(0x2B9, 0x00);
		SENSOR_I2C_WRITE(0x2BA, 0xBF);
		SENSOR_I2C_WRITE(0x2BB, 0x07);
		SENSOR_I2C_WRITE(0x2BC, 0xCF);
		SENSOR_I2C_WRITE(0x2BD, 0x07);
		SENSOR_I2C_WRITE(0x2BE, 0xCF);
		SENSOR_I2C_WRITE(0x2BF, 0x07);
		SENSOR_I2C_WRITE(0x2C0, 0xCF);
		SENSOR_I2C_WRITE(0x2C1, 0x07);
		SENSOR_I2C_WRITE(0x2C2, 0xD0);
		SENSOR_I2C_WRITE(0x2C3, 0x07);
		SENSOR_I2C_WRITE(0x2C4, 0x01);
		SENSOR_I2C_WRITE(0x2C5, 0x02);
		SENSOR_I2C_WRITE(0x2C6, 0x03);
		SENSOR_I2C_WRITE(0x2C7, 0x04);
		SENSOR_I2C_WRITE(0x2C8, 0x05);
		SENSOR_I2C_WRITE(0x2C9, 0x06);
		SENSOR_I2C_WRITE(0x2CA, 0x07);
		SENSOR_I2C_WRITE(0x2CB, 0x08);
		SENSOR_I2C_WRITE(0x2CC, 0x01);
		SENSOR_I2C_WRITE(0x2CD, 0x03);
	
		SENSOR_I2C_WRITE(0x2D1, 0x00);
		SENSOR_I2C_WRITE(0x2D2, 0x00);
		SENSOR_I2C_WRITE(0x2D3, 0x00);
		SENSOR_I2C_WRITE(0x2D4, 0x00);
		SENSOR_I2C_WRITE(0x2D5, 0x00);
		SENSOR_I2C_WRITE(0x2D6, 0x00);
		SENSOR_I2C_WRITE(0x2D7, 0x00);
		SENSOR_I2C_WRITE(0x2D8, 0x00);
		SENSOR_I2C_WRITE(0x2D9, 0x00);
		SENSOR_I2C_WRITE(0x2DA, 0x00);
		SENSOR_I2C_WRITE(0x2DB, 0x00);
		SENSOR_I2C_WRITE(0x2DC, 0x00);
		SENSOR_I2C_WRITE(0x2DD, 0x00);
		SENSOR_I2C_WRITE(0x2DE, 0x00);
		SENSOR_I2C_WRITE(0x2DF, 0x00);
		SENSOR_I2C_WRITE(0x2E0, 0x00);
		SENSOR_I2C_WRITE(0x2E1, 0x00);
		SENSOR_I2C_WRITE(0x2E2, 0x00);
		SENSOR_I2C_WRITE(0x2E3, 0x00);
		SENSOR_I2C_WRITE(0x2E4, 0x00);
		SENSOR_I2C_WRITE(0x2E5, 0x00);
		SENSOR_I2C_WRITE(0x2E6, 0x00);
		SENSOR_I2C_WRITE(0x2E7, 0x00);
		SENSOR_I2C_WRITE(0x2E8, 0x00);
		SENSOR_I2C_WRITE(0x2E9, 0x00);
		SENSOR_I2C_WRITE(0x2EA, 0x00);
		SENSOR_I2C_WRITE(0x2EB, 0x00);
		SENSOR_I2C_WRITE(0x2EC, 0x00);
		SENSOR_I2C_WRITE(0x2ED, 0x00);
		SENSOR_I2C_WRITE(0x2EE, 0x00);
		SENSOR_I2C_WRITE(0x2EF, 0x00);
		SENSOR_I2C_WRITE(0x2F0, 0x00);
		SENSOR_I2C_WRITE(0x2F1, 0x00);
		SENSOR_I2C_WRITE(0x2F2, 0x00);
		SENSOR_I2C_WRITE(0x2F3, 0x00);
		SENSOR_I2C_WRITE(0x2F4, 0x00);
		SENSOR_I2C_WRITE(0x2F5, 0x00);
		SENSOR_I2C_WRITE(0x2F6, 0x00);
		SENSOR_I2C_WRITE(0x2F7, 0x00);
		SENSOR_I2C_WRITE(0x2F8, 0x00);
		SENSOR_I2C_WRITE(0x2F9, 0x00);
		SENSOR_I2C_WRITE(0x2FA, 0x00);
		SENSOR_I2C_WRITE(0x2FB, 0x00);
		SENSOR_I2C_WRITE(0x2FC, 0x00);
		SENSOR_I2C_WRITE(0x2FD, 0x00);
		SENSOR_I2C_WRITE(0x2FE, 0x00);
		SENSOR_I2C_WRITE(0x2FF, 0x00);
	
		// chip_id = 0x3
		
		SENSOR_I2C_WRITE(0x300, 0x00);
		SENSOR_I2C_WRITE(0x301, 0x00);
		SENSOR_I2C_WRITE(0x302, 0x00);
		SENSOR_I2C_WRITE(0x303, 0x00);
		SENSOR_I2C_WRITE(0x304, 0x00);
		SENSOR_I2C_WRITE(0x305, 0x00);
		SENSOR_I2C_WRITE(0x306, 0x00);
		SENSOR_I2C_WRITE(0x307, 0xFA);
		SENSOR_I2C_WRITE(0x308, 0xFA);
		SENSOR_I2C_WRITE(0x309, 0x41);
		SENSOR_I2C_WRITE(0x30A, 0x31);
		SENSOR_I2C_WRITE(0x30B, 0x38);
		SENSOR_I2C_WRITE(0x30C, 0x04);
		SENSOR_I2C_WRITE(0x30D, 0x00);
		SENSOR_I2C_WRITE(0x30E, 0x1A);
		SENSOR_I2C_WRITE(0x30F, 0x10);
		SENSOR_I2C_WRITE(0x310, 0x00);
		SENSOR_I2C_WRITE(0x311, 0x00);
		SENSOR_I2C_WRITE(0x312, 0x10);
		SENSOR_I2C_WRITE(0x313, 0x00);
		SENSOR_I2C_WRITE(0x314, 0x00);
		SENSOR_I2C_WRITE(0x315, 0x06);
		SENSOR_I2C_WRITE(0x316, 0x33);
		SENSOR_I2C_WRITE(0x317, 0x0D);
		SENSOR_I2C_WRITE(0x318, 0x00);
		SENSOR_I2C_WRITE(0x319, 0x00);
		SENSOR_I2C_WRITE(0x31A, 0x00);
		SENSOR_I2C_WRITE(0x31B, 0x00);
		SENSOR_I2C_WRITE(0x31C, 0x00);
		SENSOR_I2C_WRITE(0x31D, 0x00);
		SENSOR_I2C_WRITE(0x31E, 0x00);
		SENSOR_I2C_WRITE(0x31F, 0x00);
		SENSOR_I2C_WRITE(0x320, 0x00);
		SENSOR_I2C_WRITE(0x321, 0x80);
		SENSOR_I2C_WRITE(0x322, 0x0C);
		SENSOR_I2C_WRITE(0x323, 0x00);
		SENSOR_I2C_WRITE(0x324, 0x00);
		SENSOR_I2C_WRITE(0x325, 0x00);
		SENSOR_I2C_WRITE(0x326, 0x00);
		SENSOR_I2C_WRITE(0x327, 0x00);
		SENSOR_I2C_WRITE(0x328, 0x05);
		SENSOR_I2C_WRITE(0x329, 0x80);
		SENSOR_I2C_WRITE(0x32A, 0x00);
		SENSOR_I2C_WRITE(0x32B, 0x00);
		SENSOR_I2C_WRITE(0x32C, 0x04);
		SENSOR_I2C_WRITE(0x32D, 0x04);
		SENSOR_I2C_WRITE(0x32E, 0x00);
		SENSOR_I2C_WRITE(0x32F, 0x00);
		SENSOR_I2C_WRITE(0x330, 0x9B);
		SENSOR_I2C_WRITE(0x331, 0x71);
		SENSOR_I2C_WRITE(0x332, 0x33);
		SENSOR_I2C_WRITE(0x333, 0x37);
		SENSOR_I2C_WRITE(0x334, 0xB3);
		SENSOR_I2C_WRITE(0x335, 0x19);
		SENSOR_I2C_WRITE(0x336, 0x97);
		SENSOR_I2C_WRITE(0x337, 0xB1);
		SENSOR_I2C_WRITE(0x338, 0x19);
		SENSOR_I2C_WRITE(0x339, 0x01);
		SENSOR_I2C_WRITE(0x33A, 0x50);
		SENSOR_I2C_WRITE(0x33B, 0x00);
		SENSOR_I2C_WRITE(0x33C, 0x35);
		SENSOR_I2C_WRITE(0x33D, 0xB0);
		SENSOR_I2C_WRITE(0x33E, 0x03);
		SENSOR_I2C_WRITE(0x33F, 0xD1);
		SENSOR_I2C_WRITE(0x340, 0x71);
		SENSOR_I2C_WRITE(0x341, 0x1D);
		SENSOR_I2C_WRITE(0x342, 0x00);
		SENSOR_I2C_WRITE(0x343, 0x00);
		SENSOR_I2C_WRITE(0x344, 0x00);
		SENSOR_I2C_WRITE(0x345, 0x00);
		SENSOR_I2C_WRITE(0x346, 0x02);
		SENSOR_I2C_WRITE(0x347, 0x30);
		SENSOR_I2C_WRITE(0x348, 0x00);
		SENSOR_I2C_WRITE(0x349, 0x00);
		SENSOR_I2C_WRITE(0x34A, 0x00);
		SENSOR_I2C_WRITE(0x34B, 0x03);
		SENSOR_I2C_WRITE(0x34C, 0x00);
		SENSOR_I2C_WRITE(0x34D, 0x02);
		SENSOR_I2C_WRITE(0x34E, 0x10);
		SENSOR_I2C_WRITE(0x34F, 0xA0);
		SENSOR_I2C_WRITE(0x350, 0x00);
		SENSOR_I2C_WRITE(0x351, 0x07);
		SENSOR_I2C_WRITE(0x352, 0x40);
		SENSOR_I2C_WRITE(0x353, 0x80);
		SENSOR_I2C_WRITE(0x354, 0x00);
		SENSOR_I2C_WRITE(0x355, 0x02);
		SENSOR_I2C_WRITE(0x356, 0x50);
		SENSOR_I2C_WRITE(0x357, 0x02);
		SENSOR_I2C_WRITE(0x358, 0x23);
		SENSOR_I2C_WRITE(0x359, 0xE4);
		SENSOR_I2C_WRITE(0x35A, 0x45);
		SENSOR_I2C_WRITE(0x35B, 0x33);
		SENSOR_I2C_WRITE(0x35C, 0x79);
		SENSOR_I2C_WRITE(0x35D, 0xD1);
		SENSOR_I2C_WRITE(0x35E, 0xCC);
		SENSOR_I2C_WRITE(0x35F, 0x2F);
		SENSOR_I2C_WRITE(0x360, 0xB6);
		SENSOR_I2C_WRITE(0x361, 0xA1);
		SENSOR_I2C_WRITE(0x362, 0x17);
		SENSOR_I2C_WRITE(0x363, 0xCB);
		SENSOR_I2C_WRITE(0x364, 0xE8);
		SENSOR_I2C_WRITE(0x365, 0xC5);
		SENSOR_I2C_WRITE(0x366, 0x32);
		SENSOR_I2C_WRITE(0x367, 0xC0);
		SENSOR_I2C_WRITE(0x368, 0xA8);
		SENSOR_I2C_WRITE(0x369, 0xC6);
		SENSOR_I2C_WRITE(0x36A, 0x5E);
		SENSOR_I2C_WRITE(0x36B, 0x20);
		SENSOR_I2C_WRITE(0x36C, 0x63);
		SENSOR_I2C_WRITE(0x36D, 0x0D);
		SENSOR_I2C_WRITE(0x36E, 0x6D);
		SENSOR_I2C_WRITE(0x36F, 0x44);
		SENSOR_I2C_WRITE(0x370, 0xA6);
		SENSOR_I2C_WRITE(0x371, 0x32);
		SENSOR_I2C_WRITE(0x372, 0x24);
		SENSOR_I2C_WRITE(0x373, 0x50);
		SENSOR_I2C_WRITE(0x374, 0xC4);
		SENSOR_I2C_WRITE(0x375, 0x2F);
		SENSOR_I2C_WRITE(0x376, 0xF4);
		SENSOR_I2C_WRITE(0x377, 0x42);
		SENSOR_I2C_WRITE(0x378, 0x82);
		SENSOR_I2C_WRITE(0x379, 0x13);
		SENSOR_I2C_WRITE(0x37A, 0x90);
		SENSOR_I2C_WRITE(0x37B, 0x00);
		SENSOR_I2C_WRITE(0x37C, 0x10);
		SENSOR_I2C_WRITE(0x37D, 0x8A);
		SENSOR_I2C_WRITE(0x37E, 0x60);
		SENSOR_I2C_WRITE(0x37F, 0xC4);
		SENSOR_I2C_WRITE(0x380, 0x2F);
		SENSOR_I2C_WRITE(0x381, 0x84);
		SENSOR_I2C_WRITE(0x382, 0xF1);
		SENSOR_I2C_WRITE(0x383, 0x0B);
		SENSOR_I2C_WRITE(0x384, 0xCD);
		SENSOR_I2C_WRITE(0x385, 0x70);
		SENSOR_I2C_WRITE(0x386, 0x42);
		SENSOR_I2C_WRITE(0x387, 0x16);
		SENSOR_I2C_WRITE(0x388, 0x00);
		SENSOR_I2C_WRITE(0x389, 0x61);
		SENSOR_I2C_WRITE(0x38A, 0x0B);
		SENSOR_I2C_WRITE(0x38B, 0x29);
		SENSOR_I2C_WRITE(0x38C, 0x74);
		SENSOR_I2C_WRITE(0x38D, 0x81);
		SENSOR_I2C_WRITE(0x38E, 0x10);
		SENSOR_I2C_WRITE(0x38F, 0xBA);
		SENSOR_I2C_WRITE(0x390, 0x18);
		SENSOR_I2C_WRITE(0x391, 0x22);
		SENSOR_I2C_WRITE(0x392, 0x11);
		SENSOR_I2C_WRITE(0x393, 0xE9);
		SENSOR_I2C_WRITE(0x394, 0x60);
		SENSOR_I2C_WRITE(0x395, 0x07);
		SENSOR_I2C_WRITE(0x396, 0x09);
		SENSOR_I2C_WRITE(0x397, 0xF6);
		SENSOR_I2C_WRITE(0x398, 0x40);
		SENSOR_I2C_WRITE(0x399, 0x02);
		SENSOR_I2C_WRITE(0x39A, 0x3C);
		SENSOR_I2C_WRITE(0x39B, 0x00);
		SENSOR_I2C_WRITE(0x39C, 0x00);
		SENSOR_I2C_WRITE(0x39D, 0x00);
		SENSOR_I2C_WRITE(0x39E, 0x00);
		SENSOR_I2C_WRITE(0x39F, 0x00);
		SENSOR_I2C_WRITE(0x3A0, 0x80);
		SENSOR_I2C_WRITE(0x3A1, 0x0B);
		SENSOR_I2C_WRITE(0x3A2, 0x64);
		SENSOR_I2C_WRITE(0x3A3, 0x90);
		SENSOR_I2C_WRITE(0x3A4, 0x8D);
		SENSOR_I2C_WRITE(0x3A5, 0x6E);
		SENSOR_I2C_WRITE(0x3A6, 0x98);
		SENSOR_I2C_WRITE(0x3A7, 0x40);
		SENSOR_I2C_WRITE(0x3A8, 0x05);
		SENSOR_I2C_WRITE(0x3A9, 0xD1);
		SENSOR_I2C_WRITE(0x3AA, 0xA8);
		SENSOR_I2C_WRITE(0x3AB, 0x86);
		SENSOR_I2C_WRITE(0x3AC, 0x09);
		SENSOR_I2C_WRITE(0x3AD, 0x54);
		SENSOR_I2C_WRITE(0x3AE, 0x10);
		SENSOR_I2C_WRITE(0x3AF, 0x8D);
		SENSOR_I2C_WRITE(0x3B0, 0x6A);
		SENSOR_I2C_WRITE(0x3B1, 0xE8);
		SENSOR_I2C_WRITE(0x3B2, 0x82);
		SENSOR_I2C_WRITE(0x3B3, 0x17);
		SENSOR_I2C_WRITE(0x3B4, 0x1C);
		SENSOR_I2C_WRITE(0x3B5, 0x60);
		SENSOR_I2C_WRITE(0x3B6, 0xC1);
		SENSOR_I2C_WRITE(0x3B7, 0x31);
		SENSOR_I2C_WRITE(0x3B8, 0xAE);
		SENSOR_I2C_WRITE(0x3B9, 0xD1);
		SENSOR_I2C_WRITE(0x3BA, 0x81);
		SENSOR_I2C_WRITE(0x3BB, 0x16);
		SENSOR_I2C_WRITE(0x3BC, 0x20);
		SENSOR_I2C_WRITE(0x3BD, 0x03);
		SENSOR_I2C_WRITE(0x3BE, 0x1B);
		SENSOR_I2C_WRITE(0x3BF, 0x24);
		SENSOR_I2C_WRITE(0x3C0, 0xE0);
		SENSOR_I2C_WRITE(0x3C1, 0xC1);
		SENSOR_I2C_WRITE(0x3C2, 0x33);
		SENSOR_I2C_WRITE(0x3C3, 0xBE);
		SENSOR_I2C_WRITE(0x3C4, 0x51);
		SENSOR_I2C_WRITE(0x3C5, 0x82);
		SENSOR_I2C_WRITE(0x3C6, 0x1E);
		SENSOR_I2C_WRITE(0x3C7, 0x40);
		SENSOR_I2C_WRITE(0x3C8, 0x03);
		SENSOR_I2C_WRITE(0x3C9, 0x1C);
		SENSOR_I2C_WRITE(0x3CA, 0x34);
		SENSOR_I2C_WRITE(0x3CB, 0xD0);
		SENSOR_I2C_WRITE(0x3CC, 0x81);
		SENSOR_I2C_WRITE(0x3CD, 0x02);
		SENSOR_I2C_WRITE(0x3CE, 0x16);
		SENSOR_I2C_WRITE(0x3CF, 0x00);
		SENSOR_I2C_WRITE(0x3D0, 0x02);
		SENSOR_I2C_WRITE(0x3D1, 0x04);
		SENSOR_I2C_WRITE(0x3D2, 0x00);
		SENSOR_I2C_WRITE(0x3D3, 0x00);
		SENSOR_I2C_WRITE(0x3D4, 0x00);
		SENSOR_I2C_WRITE(0x3D5, 0x80);
		SENSOR_I2C_WRITE(0x3D6, 0x00);
		SENSOR_I2C_WRITE(0x3D7, 0x00);
		SENSOR_I2C_WRITE(0x3D8, 0x23);
		SENSOR_I2C_WRITE(0x3D9, 0x01);
		SENSOR_I2C_WRITE(0x3DA, 0x03);
		SENSOR_I2C_WRITE(0x3DB, 0x02);
		SENSOR_I2C_WRITE(0x3DC, 0x00);
		SENSOR_I2C_WRITE(0x3DD, 0x00);
		SENSOR_I2C_WRITE(0x3DE, 0x00);
		SENSOR_I2C_WRITE(0x3DF, 0x00);
		SENSOR_I2C_WRITE(0x3E0, 0x22);
		SENSOR_I2C_WRITE(0x3E1, 0x00);
		SENSOR_I2C_WRITE(0x3E2, 0x00);
		SENSOR_I2C_WRITE(0x3E3, 0x00);
		SENSOR_I2C_WRITE(0x3E4, 0x3F);
		SENSOR_I2C_WRITE(0x3E5, 0x17);
		SENSOR_I2C_WRITE(0x3E6, 0x15);
		SENSOR_I2C_WRITE(0x3E7, 0x00);
		SENSOR_I2C_WRITE(0x3E8, 0x00);
		SENSOR_I2C_WRITE(0x3E9, 0x00);
		SENSOR_I2C_WRITE(0x3EA, 0x00);
		SENSOR_I2C_WRITE(0x3EB, 0x00);
		SENSOR_I2C_WRITE(0x3EC, 0x00);
		SENSOR_I2C_WRITE(0x3ED, 0x00);
		SENSOR_I2C_WRITE(0x3EE, 0x00);
		SENSOR_I2C_WRITE(0x3EF, 0x00);
		SENSOR_I2C_WRITE(0x3F0, 0x00);
		SENSOR_I2C_WRITE(0x3F1, 0x00);
		SENSOR_I2C_WRITE(0x3F2, 0x00);
		SENSOR_I2C_WRITE(0x3F3, 0x00);
		SENSOR_I2C_WRITE(0x3F4, 0x00);
		SENSOR_I2C_WRITE(0x3F5, 0x00);
		SENSOR_I2C_WRITE(0x3F6, 0x00);
		SENSOR_I2C_WRITE(0x3F7, 0x00);
		SENSOR_I2C_WRITE(0x3F8, 0x00);
		SENSOR_I2C_WRITE(0x3F9, 0x00);
		SENSOR_I2C_WRITE(0x3FA, 0x00);
		SENSOR_I2C_WRITE(0x3FB, 0x00);
		SENSOR_I2C_WRITE(0x3FC, 0x00);
		SENSOR_I2C_WRITE(0x3FD, 0x00);
		SENSOR_I2C_WRITE(0x3FE, 0x00);
		SENSOR_I2C_WRITE(0x3FF, 0x00);
		
		// standby cancel
	//	SENSOR_I2C_WRITE(0x200, 0x30);
	
		// waiting for internal regular stabilization
		//usleep(200000);
		
		
		  // SENSOR_I2C_WRITE(0x226, 0x00);
		   
		   // standby cancel
		   SENSOR_I2C_WRITE(0x200, 0x30);
		// XVS,XHS output start
		//SENSOR_I2C_WRITE(0x229, 0xC0);
	
		// waiting for image stabilization
		usleep(200000);

		memset(&sensor_exp_func, 0, sizeof(sensor_exp_func));
		sensor_exp_func.pfn_cmos_inttime_initialize = imx122_inttime_initialize;
	    sensor_exp_func.pfn_cmos_inttime_update = imx122_inttime_update;

	    sensor_exp_func.pfn_cmos_gains_initialize = imx122_gains_initialize;
	    sensor_exp_func.pfn_cmos_gains_update = imx122_gains_update;
	    sensor_exp_func.pfn_cmos_gains_update2 = NULL;
	    sensor_exp_func.pfn_analog_gain_from_exposure_calculate = imx122_analog_gain_from_exposure_calculate;
	    sensor_exp_func.pfn_digital_gain_from_exposure_calculate = imx122_digital_gain_from_exposure_calculate;

	    sensor_exp_func.pfn_cmos_fps_set = imx122_fps_set;
	    sensor_exp_func.pfn_vblanking_calculate = imx122_vblanking_calculate;
	    sensor_exp_func.pfn_cmos_vblanking_front_update = imx122_vblanking_update;

	    sensor_exp_func.pfn_setup_sensor = imx122_setup_sensor;

	    sensor_exp_func.pfn_cmos_get_analog_gain = imx122_get_analog_gain;
	    sensor_exp_func.pfn_cmos_get_digital_gain = imx122_get_digital_gain;
	    sensor_exp_func.pfn_cmos_get_digital_fine_gain = NULL;
	    sensor_exp_func.pfn_cmos_get_iso = imx122_get_ISO;

	    sensor_exp_func.pfn_cmos_get_isp_default = imx122_get_isp_default;
	    sensor_exp_func.pfn_cmos_get_isp_special_alg = NULL;
	   	sensor_exp_func.pfn_cmos_get_isp_agc_table = imx122_get_isp_agc_table;
		sensor_exp_func.pfn_cmos_get_isp_noise_table = imx122_get_isp_noise_table;
		sensor_exp_func.pfn_cmos_get_isp_demosaic = imx122_get_isp_demosaic;
		sensor_exp_func.pfn_cmos_get_isp_shading_table = NULL;

		SOC_CHECK(HI_MPI_ISP_SensorRegCallBack(&sensor_exp_func));
		printf("-------Sony IMX122 Sensor Initial OK!-------\n");
}



