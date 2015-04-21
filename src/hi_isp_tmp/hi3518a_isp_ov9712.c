
#include "sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_OV9712_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_OV9712_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)
   
static cmos_inttime_t cmos_inttime;
static cmos_gains_t cmos_gains;
                                    
#define CMOS_OV9712_ISP_WRITE_SENSOR_ENABLE (1)

/*change this value to 1 to make the image looks more sharpen but with more white edge*/    
#define CMOS_OV9712_MORE_SHARPEN (0)

static cmos_isp_default_t st_coms_isp_default =
{
	// color matrix[9]
    {
        5048,
        {   0x021f, 0x80c7, 0x8058,
            0x8041, 0x1b2, 0x8071,
            0xa, 0x8134, 0x229
        },
        3200,
        {   0x1d7, 0x805d, 0x807a,
            0x8076, 0x1eb, 0x8075,
            0x8029, 0x8178, 0x02a1
        },
        2480,
        {   0x248, 0x80dd, 0x806b,
            0x8055, 0x1a1, 0x804c,
            0x8038, 0x827e, 0x3b6
        }
    },


    // black level
   // {25,24,21,21},
    	{31,16,16,38},

    //calibration reference color temperature
    5000,

    //WB gain at 5000, must keep consistent with calibration color temperature
    {0x017a, 0x100, 0x100, 0x0189},

    // WB curve parameters, must keep consistent with reference color temperature.
    {148, -68, -176, 224061, 128, -175798},

	// hist_thresh
	{0xd,0x28,0x60,0x80},
    //{0x10,0x40,0xc0,0xf0},

	0x0,	// iridix_balck
	0x3,	// bggr

	/* limit max gain for reducing noise,    */
	0x1f,	0x1,

	// iridix
	0x04,	0x08,	0xa0, 	0x4ff,

	0x1, 	// balance_fe
	0x78,	// ae compensation
	0x15, 	// sinter threshold

	0x0,  0,  0  //noise profile=0, use the default noise profile lut, don't need to set nr0 and nr1
};

static cmos_isp_agc_table_t st_isp_agc_table =
{
#if CMOS_OV9712_MORE_SHARPEN
    //sharpen_alt_d
    {0xc3,0xc2,0xc1,0xbc,0xa0,0x83,0x83,0x83},

    //sharpen_alt_ud
    {0xce,0xc5,0xba,0x7c,0x5d,0x4f,0x4f,0x4f},

    //snr_thresh
    {0x19,0x21,0x31,0x37,0x41,0x4a,0x4a,0x4a},
#else    
    //sharpen_alt_d
    {0xae,0x9b,0x7e,0x58,0x30,0x76,0x76,0x76},

    //sharpen_alt_ud
    {0xb0,0xa0,0x7e,0x58,0x44,0x3c,0x3c,0x3c},

    //snr_thresh
    {0x14,0x1e,0x2d,0x32,0x39,0x3f,0x3f,0x3f},
#endif

    //demosaic_lum_thresh
    {0x40,0x60,0x80,0x80,0x80,0x80,0x80,0x80},

    //demosaic_np_offset
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

    //ge_strength
    {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55},

    /* saturation */
    {0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}

};

static cmos_isp_noise_table_t st_isp_noise_table =
{
    //nosie_profile_weight_lut
    {0, 27, 31, 33, 35, 36, 37, 38, 39, 40, 40, 41, 41, 42, 42, 43,
    43, 43, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47,
    47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49,
    49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51,
    51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55},

    //demosaic_weight_lut
    {0, 27, 31, 33, 35, 36, 37, 38, 39, 40, 40, 41, 41, 42, 42, 43,
    43, 43, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47,
    47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49,
    49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51,
    51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55}
};

static cmos_isp_demosaic_t st_isp_demosaic =
{
    /*vh_slope*/
    0xda,

    /*aa_slope*/
    0xa9,

    /*va_slope*/
    0xec,

    /*uu_slope*/
 #if CMOS_OV9712_MORE_SHARPEN
    0x9e,
#else
    0x84,
#endif

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0xa9,

    /*aa_thresh*/
    0x23,

    /*va_thresh*/
    0xa6,

    /*uu_thresh*/
    0x2d,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};

static cmos_isp_shading_table_t st_isp_shading_table =
{
    /*shading_center_r*/
    0x27a, 0x164,

    /*shading_center_g*/
    0x277, 0x16c,

    /*shading_center_b*/
    0x27c, 0x168,

    /*shading_table_r*/
    {0x1000,0x1000,0x1007,0x101b,0x1035,0x1052,0x1071,0x1091,0x10b2,0x10d4,0x10f6,0x1118,
	0x113b,0x115e,0x1180,0x11a3,0x11c6,0x11e9,0x120c,0x122f,0x1251,0x1274,0x1296,0x12b8,
	0x12da,0x12fb,0x131e,0x133f,0x1360,0x1382,0x13a2,0x13c2,0x13e3,0x1404,0x1424,0x1443,
	0x1463,0x1482,0x14a1,0x14c0,0x14de,0x14fd,0x151a,0x1538,0x1556,0x1573,0x158f,0x15ac,
	0x15c9,0x15e5,0x1601,0x161c,0x1636,0x1652,0x166c,0x1687,0x16a1,0x16bb,0x16d3,0x16ed,
	0x1706,0x171f,0x1737,0x174f,0x1766,0x177d,0x1795,0x17ab,0x17c3,0x17d9,0x17ef,0x1804,
	0x181a,0x182f,0x1844,0x1858,0x186d,0x1881,0x1894,0x18a7,0x18bb,0x18ce,0x18e1,0x18f3,
	0x1905,0x1917,0x1929,0x193a,0x194b,0x195b,0x196d,0x197e,0x198d,0x199e,0x19ad,0x19bd,
	0x19cc,0x19dc,0x19ea,0x19f9,0x1a08,0x1a16,0x1a24,0x1a32,0x1a40,0x1a4e,0x1a5a,0x1a69,
	0x1a76,0x1a83,0x1a8f,0x1a9c,0x1aa9,0x1ab5,0x1ac1,0x1acc,0x1ad9,0x1ae5,0x1af1,0x1afc,
	0x1b08,0x1b12,0x1b1f,0x1b2a,0x1b35,0x1b40,0x1b4b,0x1b56,0x1b5f},

    /*shading_table_g*/
    {0x1000,0x1000,0x1004,0x1015,0x102d,0x1047,0x1064,0x1082,0x10a1,0x10c0,0x10e0,0x1101,
	0x1122,0x1143,0x1164,0x1185,0x11a7,0x11c8,0x11e9,0x120a,0x122b,0x124c,0x126d,0x128e,
	0x12ae,0x12cf,0x12ef,0x130f,0x132f,0x134e,0x136e,0x138d,0x13ac,0x13cb,0x13e9,0x1407,
	0x1425,0x1443,0x1461,0x147e,0x149b,0x14b7,0x14d3,0x14f0,0x150b,0x1527,0x1542,0x155d,
	0x1577,0x1591,0x15ab,0x15c4,0x15de,0x15f6,0x160f,0x1627,0x163f,0x1656,0x166e,0x1684,
	0x169b,0x16b1,0x16c7,0x16dc,0x16f1,0x1706,0x171a,0x172e,0x1741,0x1755,0x1768,0x177a,
	0x178c,0x179e,0x17b0,0x17c1,0x17d1,0x17e2,0x17f2,0x1801,0x1811,0x1820,0x182e,0x183d,
	0x184a,0x1858,0x1865,0x1872,0x187f,0x188b,0x1897,0x18a2,0x18ae,0x18b9,0x18c3,0x18cd,
	0x18d7,0x18e1,0x18ea,0x18f3,0x18fc,0x1904,0x190d,0x1914,0x191c,0x1923,0x192a,0x1931,
	0x1937,0x193d,0x1943,0x1949,0x194e,0x1953,0x1958,0x195d,0x1961,0x1965,0x1969,0x196d,
	0x1970,0x1973,0x1976,0x1979,0x197c,0x197e,0x1980,0x1982,0x1984},

    /*shading_table_b*/
    {0x1000,0x1000,0x1000,0x1006,0x1011,0x101f,0x1030,0x1042,0x1054,0x1068,0x107c,0x1091,\
	0x10a6,0x10bc,0x10d2,0x10e7,0x10fd,0x1114,0x112a,0x1140,0x1156,0x116d,0x1183,0x119a,\
	0x11b0,0x11c6,0x11dc,0x11f3,0x1209,0x121f,0x1235,0x124b,0x1261,0x1276,0x128c,0x12a2,\
	0x12b7,0x12cc,0x12e2,0x12f7,0x130c,0x1320,0x1335,0x134a,0x135e,0x1372,0x1386,0x139a,\
	0x13ae,0x13c1,0x13d5,0x13e8,0x13fb,0x140e,0x1421,0x1433,0x1445,0x1457,0x1469,0x147b,\
	0x148c,0x149e,0x14af,0x14bf,0x14d0,0x14e0,0x14f1,0x1501,0x1510,0x1520,0x152f,0x153e,\
	0x154d,0x155b,0x156a,0x1578,0x1586,0x1593,0x15a0,0x15ad,0x15ba,0x15c7,0x15d3,0x15df,\
	0x15eb,0x15f6,0x1602,0x160d,0x1617,0x1622,0x162c,0x1636,0x1640,0x1649,0x1652,0x165b,\
	0x1664,0x166c,0x1674,0x167c,0x1683,0x168a,0x1691,0x1698,0x169e,0x16a4,0x16aa,0x16b0,\
	0x16b5,0x16ba,0x16bf,0x16c3,0x16c8,0x16cc,0x16cf,0x16d3,0x16d6,0x16d9,0x16db,0x16de,\
	0x16e0,0x16e2,0x16e3,0x16e4,0x16e5,0x16e6,0x16e7,0x16e7,0x16e7},

    /*shading_off_center_r_g_b*/
    0xf8e, 0xf8e, 0xf8e,

    /*shading_table_nobe_number*/
    129
};

/*
 * This function initialises an instance of cmos_inttime_t.
 */
static cmos_inttime_const_ptr_t ov9712_inttime_initialize()
{
	cmos_inttime.full_lines_std = 810;
	cmos_inttime.full_lines_std_30fps = 810;
	cmos_inttime.full_lines = 810;
	cmos_inttime.full_lines_limit = 65535;
    cmos_inttime.max_lines = 806;
    cmos_inttime.min_lines = 2;
	cmos_inttime.max_lines_target = cmos_inttime.max_lines;
	cmos_inttime.min_lines_target = cmos_inttime.min_lines;

	cmos_inttime.vblanking_lines = 0;

	cmos_inttime.exposure_ashort = 0;
	cmos_inttime.exposure_shift = 0;

	cmos_inttime.lines_per_500ms = cmos_inttime.full_lines_std_30fps * 30 / 2; 
	cmos_inttime.flicker_freq = 0;//60*256;//50*256;

	return &cmos_inttime;
}

/*
 * This function applies the new integration time to the ISP registers.
 */
static void ov9712_inttime_update(cmos_inttime_ptr_t p_inttime)
{
#if CMOS_OV9712_ISP_WRITE_SENSOR_ENABLE
		ISP_I2C_DATA_S stI2cData;
		
		stI2cData.bDelayCfg = HI_FALSE;
		stI2cData.u8DevAddr = 0x60;
		stI2cData.u32AddrByteNum = 1;
		stI2cData.u32DataByteNum = 1;
		stI2cData.u32RegAddr = 0x10;
		stI2cData.u32Data = p_inttime->exposure_ashort & 0xFF;
		HI_MPI_ISP_I2cWrite(&stI2cData);
		
		stI2cData.u32RegAddr = 0x16;
		stI2cData.u32Data = (p_inttime->exposure_ashort >> 8) & 0xFF;
		HI_MPI_ISP_I2cWrite(&stI2cData);
		
#else

	HI_U32 _curr = p_inttime->exposure_ashort;

    _curr = ((_curr >= 760) && (_curr < 768))? 768: _curr;
 
    //refresh the sensor setting every frame to avoid defect pixel error
    SENSOR_I2C_WRITE(0x10, _curr&0xFF);
    SENSOR_I2C_WRITE(0x16, (_curr>>8)&0xFF);
#endif
    return;
}

/*
 * This function applies the new vert blanking porch to the ISP registers.
 */
static void ov9712_vblanking_update(cmos_inttime_const_ptr_t p_inttime)
{
    SENSOR_I2C_WRITE(0x2d, p_inttime->vblanking_lines & 0xff);
    SENSOR_I2C_WRITE(0x2e, (p_inttime->vblanking_lines & 0xff00) >> 8);

    return;
}

static HI_U16 ov9712_vblanking_calculate(
		cmos_inttime_ptr_t p_inttime)
{
	if(p_inttime->exposure_ashort >= p_inttime->full_lines - 4)
	{
		p_inttime->exposure_ashort = p_inttime->full_lines - 4;
	}

	p_inttime->vblanking_lines = p_inttime->full_lines - p_inttime->full_lines_std_30fps;

	return p_inttime->exposure_ashort;
}

/* Set fps base */
static void ov9712_fps_set(
		cmos_inttime_ptr_t p_inttime,
		const HI_U8 fps
		)
{
	switch(fps)
	{
		case 30:
            p_inttime->lines_per_500ms = cmos_inttime.full_lines_std_30fps * 30 / 2; 
            SENSOR_I2C_WRITE(0x2a, 0x98);
            SENSOR_I2C_WRITE(0x2b, 0x6);
		break;

		case 25:
            /* do not change full_lines_std */
			p_inttime->lines_per_500ms = cmos_inttime.full_lines_std_30fps * 25 / 2;
            SENSOR_I2C_WRITE(0x2a, 0xe8);
            SENSOR_I2C_WRITE(0x2b, 0x7);
        break;

		default:
		break;
	}
    
	return;
}

/*
 * This function initialises an instance of cmos_gains_t.
 */
static cmos_gains_ptr_t ov9712_gains_initialize()
{
    cmos_gains.max_again = 496;
	cmos_gains.max_dgain = 1;

	cmos_gains.again_shift = 4;
	cmos_gains.dgain_shift = 0;
	cmos_gains.dgain_fine_shift = 0;

    cmos_gains.isp_dgain_shift = 4;
    cmos_gains.isp_dgain = 1 << cmos_gains.isp_dgain_shift;
    cmos_gains.max_isp_dgain_target = 1 << cmos_gains.isp_dgain_shift;

	return &cmos_gains;
}

static HI_U32 ov9712_get_ISO(cmos_gains_ptr_t p_gains)
{
	HI_U32 _again = p_gains->again == 0 ? 1 : p_gains->again;
	HI_U32 _dgain = p_gains->dgain == 0 ? 1 : p_gains->dgain;

	p_gains->iso =  ((_again * _dgain * 100) >> (p_gains->again_shift + p_gains->dgain_shift));

	return p_gains->iso;
}

/*
 * This function applies the new gains to the ISP registers.
 */
static void ov9712_gains_update(cmos_gains_const_ptr_t p_gains)
{
#if CMOS_OV9712_ISP_WRITE_SENSOR_ENABLE
		ISP_I2C_DATA_S stI2cData;
	
		stI2cData.bDelayCfg = HI_TRUE;
		stI2cData.u8DevAddr = 0x60;
		stI2cData.u32AddrByteNum = 1;
		stI2cData.u32DataByteNum = 1;
		stI2cData.u32RegAddr = 0x00;
		stI2cData.u32Data = p_gains->again_db;
		HI_MPI_ISP_I2cWrite(&stI2cData);
		
#else

    SENSOR_I2C_WRITE(0x00, p_gains->again_db);
#endif
	return;
}

static HI_U32 ov9712_analog_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift)
{
    HI_U32 _again = 1 << p_gains->again_shift;
    HI_U32 _again_new;
    int shift = 0;
    HI_U8 i,j;

    while (exposure > (1<<20))
    {
        exposure >>= 1;
        exposure_max >>= 1;
        ++shift;
    }

    for(i = 0;i < 5; i++)
    {
        for(j = 0; j < 16; j++)
        {
            _again_new = (16 + j) << i;

            if(_again_new > p_gains->max_again_target)
            {                
                goto AGAIN_CALCULATE_LOOP_END;
            }

            _again = _again_new;

            if((_again >= p_gains->min_again_target) && (((exposure_max * _again) >> 4) >= exposure))
            {
                goto AGAIN_CALCULATE_LOOP_END;
            }
        }
    }

    /* revert i&j to their max values */
    i--;
    j--;
    AGAIN_CALCULATE_LOOP_END:

    p_gains->again = _again;
    p_gains->again_db = (1 << (i + 4)) + j - 16;

	return (exposure << (shift + 4)) / _again;
    
}

static HI_U32 ov9712_digital_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift)
{
	p_gains->dgain = 1;

	return exposure;
}

static HI_U32 ov9712_get_isp_agc_table(cmos_isp_agc_table_ptr_t p_cmos_isp_agc_table)
{
	if (NULL == p_cmos_isp_agc_table)
	{
	    printf("null pointer when get isp agc table value!\n");
	    return -1;
	}
    memcpy(p_cmos_isp_agc_table, &st_isp_agc_table, sizeof(cmos_isp_agc_table_t));
    return 0;
}

static HI_U32 ov9712_get_isp_noise_table(cmos_isp_noise_table_ptr_t p_cmos_isp_noise_table)
{
	if (NULL == p_cmos_isp_noise_table)
	{
	    printf("null pointer when get isp noise table value!\n");
	    return -1;
	}
    memcpy(p_cmos_isp_noise_table, &st_isp_noise_table, sizeof(cmos_isp_noise_table_t));
    return 0;
}

static HI_U32 ov9712_get_isp_demosaic(cmos_isp_demosaic_ptr_t p_cmos_isp_demosaic)
{
   if (NULL == p_cmos_isp_demosaic)
   {
	    printf("null pointer when get isp demosaic value!\n");
	    return -1;
   }
   memcpy(p_cmos_isp_demosaic, &st_isp_demosaic,sizeof(cmos_isp_demosaic_t));
   return 0;

}

static HI_U32 ov9712_get_isp_shading_table(cmos_isp_shading_table_ptr_t p_cmos_isp_shading_table)
{
	if (NULL == p_cmos_isp_shading_table)
	{
	    printf("null pointer when get isp shading table value!\n");
	    return -1;
	}
    memcpy(p_cmos_isp_shading_table, &st_isp_shading_table, sizeof(cmos_isp_shading_table_t));
    return 0;
}


static void ov9712_setup_sensor(int isp_mode)
{
	if(0 == isp_mode) /* ISP 'normal' isp_mode */
	{
        SENSOR_I2C_WRITE(0x2d, 0x0);
        SENSOR_I2C_WRITE(0x2e, 0x0);
	}
	else if(1 == isp_mode) /* ISP pixel calibration isp_mode */
	{
        /* 5 fps */
        SENSOR_I2C_WRITE(0x2d, 0xd2); 
        SENSOR_I2C_WRITE(0x2e, 0x0f); 
        
        /* min gain */
        SENSOR_I2C_WRITE(0x0, 0x00);               

        /* max exposure time*/
        SENSOR_I2C_WRITE(0x10, 0xf8);
    	SENSOR_I2C_WRITE(0x16, 0x12);
	}
}

static HI_U32 ov9712_gains_lin_to_db_convert(HI_U32 data, HI_U32 shift_in)
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

static HI_U8 ov9712_get_analog_gain(cmos_gains_ptr_t p_gains)
{
    return ov9712_gains_lin_to_db_convert(p_gains->again, p_gains->again_shift);
}

static HI_U8 ov9712_get_digital_gain(cmos_gains_ptr_t p_gains)
{
    return 0;
}

static HI_U32 ov9712_get_isp_default(cmos_isp_default_ptr_t p_coms_isp_default)
{
	if (NULL == p_coms_isp_default)
	{
	    printf("null pointer when get isp default value!\n");
	    return -1;
	}
    memcpy(p_coms_isp_default, &st_coms_isp_default, sizeof(cmos_isp_default_t));
    return 0;
}



void OV9712_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write)
{
	SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	// [720p @ 30fps]
	//Reset
	SENSOR_I2C_WRITE(0x12, 0x80);
	SENSOR_I2C_WRITE(0x09, 0x10);

	//Core Settings
	SENSOR_I2C_WRITE(0x1e, 0x07);
	SENSOR_I2C_WRITE(0x5f, 0x18);
	SENSOR_I2C_WRITE(0x69, 0x04);
	SENSOR_I2C_WRITE(0x65, 0x2a);
	SENSOR_I2C_WRITE(0x68, 0x0a);
	SENSOR_I2C_WRITE(0x39, 0x28);
	SENSOR_I2C_WRITE(0x4d, 0x90);
	SENSOR_I2C_WRITE(0xc1, 0x80);
	SENSOR_I2C_WRITE(0x0c, 0x30);
	SENSOR_I2C_WRITE(0x6d, 0x02);

	//DSP
	//SENSOR_I2C_WRITE(0x96, 0xf1);
	SENSOR_I2C_WRITE(0x96, 0x01);
	SENSOR_I2C_WRITE(0xbc, 0x68);

	//Resolution and Format
	SENSOR_I2C_WRITE(0x12, 0x00);
	SENSOR_I2C_WRITE(0x3b, 0x00);
	SENSOR_I2C_WRITE(0x97, 0x80);
	SENSOR_I2C_WRITE(0x17, 0x25);
	SENSOR_I2C_WRITE(0x18, 0xA2);
	SENSOR_I2C_WRITE(0x19, 0x01);
	SENSOR_I2C_WRITE(0x1a, 0xCA);
	SENSOR_I2C_WRITE(0x03, 0x0A);
	SENSOR_I2C_WRITE(0x32, 0x07);
	SENSOR_I2C_WRITE(0x98, 0x00);
	SENSOR_I2C_WRITE(0x99, 0x28);
	SENSOR_I2C_WRITE(0x9a, 0x00);
	SENSOR_I2C_WRITE(0x57, 0x00);
	SENSOR_I2C_WRITE(0x58, 0xB4);
	SENSOR_I2C_WRITE(0x59, 0xA0);
	SENSOR_I2C_WRITE(0x4c, 0x13);
	SENSOR_I2C_WRITE(0x4b, 0x36);
	SENSOR_I2C_WRITE(0x3d, 0x3c);
	SENSOR_I2C_WRITE(0x3e, 0x03);
	SENSOR_I2C_WRITE(0xbd, 0xA0);
	SENSOR_I2C_WRITE(0xbe, 0xb4);

	//YAVG
	SENSOR_I2C_WRITE(0x4e, 0x55);
	SENSOR_I2C_WRITE(0x4f, 0x55);
	SENSOR_I2C_WRITE(0x50, 0x55);
	SENSOR_I2C_WRITE(0x51, 0x55);
	SENSOR_I2C_WRITE(0x24, 0x55);
	SENSOR_I2C_WRITE(0x25, 0x40);
	SENSOR_I2C_WRITE(0x26, 0xa1);

	//Clock
	SENSOR_I2C_WRITE(0x5c, 0x52);
	SENSOR_I2C_WRITE(0x5d, 0x00);
	SENSOR_I2C_WRITE(0x11, 0x01);
	SENSOR_I2C_WRITE(0x2a, 0x98);
	SENSOR_I2C_WRITE(0x2b, 0x06);
	SENSOR_I2C_WRITE(0x2d, 0x00);
	SENSOR_I2C_WRITE(0x2e, 0x00);

	//General
	SENSOR_I2C_WRITE(0x13, 0xA5);
	SENSOR_I2C_WRITE(0x14, 0x40);

	//Banding
	SENSOR_I2C_WRITE(0x4a, 0x00);
	SENSOR_I2C_WRITE(0x49, 0xce);
	SENSOR_I2C_WRITE(0x22, 0x03);
	SENSOR_I2C_WRITE(0x09, 0x00);

	//close AE_AWB
	SENSOR_I2C_WRITE(0x13, 0x80);
	SENSOR_I2C_WRITE(0x16, 0x00);
	SENSOR_I2C_WRITE(0x10, 0xf0);
	SENSOR_I2C_WRITE(0x00, 0x3f);
	SENSOR_I2C_WRITE(0x38, 0x00);
	SENSOR_I2C_WRITE(0x01, 0x40);
	SENSOR_I2C_WRITE(0x02, 0x40);
	SENSOR_I2C_WRITE(0x05, 0x40);
	SENSOR_I2C_WRITE(0x06, 0x00);
	SENSOR_I2C_WRITE(0x07, 0x00);

    //BLC
    SENSOR_I2C_WRITE(0x41, 0x84);

	memset(&sensor_exp_func, 0, sizeof(sensor_exp_func));	
	sensor_exp_func.pfn_cmos_inttime_initialize = ov9712_inttime_initialize;
    sensor_exp_func.pfn_cmos_inttime_update = ov9712_inttime_update;

    sensor_exp_func.pfn_cmos_gains_initialize = ov9712_gains_initialize;
    sensor_exp_func.pfn_cmos_gains_update = ov9712_gains_update;
    sensor_exp_func.pfn_cmos_gains_update2 = NULL;
   	sensor_exp_func.pfn_analog_gain_from_exposure_calculate = ov9712_analog_gain_from_exposure_calculate;
    sensor_exp_func.pfn_digital_gain_from_exposure_calculate = ov9712_digital_gain_from_exposure_calculate;

    sensor_exp_func.pfn_cmos_fps_set = ov9712_fps_set;
    sensor_exp_func.pfn_vblanking_calculate = ov9712_vblanking_calculate;
    sensor_exp_func.pfn_cmos_vblanking_front_update = ov9712_vblanking_update;

    sensor_exp_func.pfn_setup_sensor = ov9712_setup_sensor;

	sensor_exp_func.pfn_cmos_get_analog_gain = ov9712_get_analog_gain;
	sensor_exp_func.pfn_cmos_get_digital_gain = ov9712_get_digital_gain;
	sensor_exp_func.pfn_cmos_get_digital_fine_gain = NULL;
    sensor_exp_func.pfn_cmos_get_iso = ov9712_get_ISO;

	sensor_exp_func.pfn_cmos_get_isp_default = ov9712_get_isp_default;
	sensor_exp_func.pfn_cmos_get_isp_special_alg = NULL;
	sensor_exp_func.pfn_cmos_get_isp_agc_table = ov9712_get_isp_agc_table,
	sensor_exp_func.pfn_cmos_get_isp_noise_table = ov9712_get_isp_noise_table;
	sensor_exp_func.pfn_cmos_get_isp_demosaic = ov9712_get_isp_demosaic;
	sensor_exp_func.pfn_cmos_get_isp_shading_table = ov9712_get_isp_shading_table;
	SOC_CHECK(HI_MPI_ISP_SensorRegCallBack(&sensor_exp_func));

	printf("OV9712 sensor 720P30fps init success!\n");
		
}


