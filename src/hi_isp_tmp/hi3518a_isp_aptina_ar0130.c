
#include "sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_APTINA_AR0130_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_APTINA_AR0130_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

static cmos_isp_default_t st_coms_isp_default =
{
    {
        5048,
        {
            0x206,0x80b9,0x804d,
            0x8046,0x163,0x801d,
            0x7,  0x80c5,0x1bd
        },

        3200,
        {
            0x1e1,0x807a,0x8056,
            0x807a,0x187,0x800d,
            0x8012,0x8112,0x224
        },

        2480,
        {
         0x1ec,  0x80a2, 0x804a,
         0x807c, 0x166,  0x15,
         0x806d, 0x81c8, 0x335
        }
    },

	// black level for R, Gr, Gb, B channels
	{0xa7,0xa7,0xa6,0xa7},

	//calibration reference color temperature
	5048,

	//WB gain at Macbeth 5000K, must keep consistent with calibration color temperature

    // {0x181,0x100,0x100,0x1b3}, //for demo
    {0x17b,0x0100,0x0100,0x1ae},//for ref

	// WB curve parameters, must keep consistent with reference color temperature.

    // {96,-23,-182,225343,128,-178914}, //for demo
    {109,-53,-200,172800,128,-125823},  //for ref


	// hist_thresh
	{0xd,0x28,0x60,0x80},

	0x00,	// iridix_balck
	0x1,	// rggb

	// gain
	//0x8,	0x8, // this is gain target, it will be constricted by sensor-gain.
	0x8,	0x4, /* The purpose of setting max dgain target to 4 is to reduce FPN */

	//wdr_variance_space, wdr_variance_intensity, slope_max_write,  white_level_write
	0x04,	0x01,	0x30, 	0x4FF,

	0x1, 	// balance_fe
	0x80,	// ae compensation
	0x23, 	// sinter threshold

	0x1,     //noise profile=0, use the default noise profile lut, don't need to set nr0 and nr1
	0x0,
	546
};

static cmos_isp_agc_table_t st_isp_agc_table =
{
    //sharpen_alt_d
    {0x70,0x68,0x40,0x38,0x34,0x30,0x28,0x28},

    //sharpen_alt_ud
    {0xa0,0x98,0x80,0x70,0x60,0x50,0x40,0x40},

    //snr_thresh
    {0x13,0x19,0x20,0x26,0x2c,0x32,0x38,0x38},

    //demosaic_lum_thresh
    {0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80},

    //demosaic_np_offset
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

    //ge_strength
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37},

    /* saturation */
    {0x80,0x80,0x6C,0x48,0x44,0x40,0x3C,0x38}
};


static cmos_isp_noise_table_t st_isp_noise_table =
{
  //nosie_profile_weight_lut
    {0,  0,  0,  0,  0,  0,  11, 15, 17, 19, 20, 21, 22, 22, 23, 24,
    25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29,
    30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32,
    32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34,
    34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38},

  //demosaic_weight_lut
    {0,11,15,17,19,20,21,22,23,23,24,25,25,26,26,26,
    27,27,27,28,28,28,29,29,29,29,29,30,30,30,30,30,
    31,31,31,31,31,32,32,32,32,32,32,32,33,33,33,33,
    33,33,33,33,33,34,34,34,34,34,34,34,34,34,34,35,
    35,35,35,35,35,35,35,35,35,35,35,36,36,36,36,36,
    36,36,36,36,36,36,36,36,36,37,37,37,37,37,37,37,
    37,37,37,37,37,37,37,37,37,38,38,38,38,38,38,38,
    38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38}
};

static cmos_isp_demosaic_t st_isp_demosaic =
{
    /*vh_slope*/
    0xf5,

    /*aa_slope*/
    0xb4,

    /*va_slope*/
    0xe6,

    /*uu_slope*/
    0x80,

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0x30,

    /*aa_thresh*/
    0x5b,

    /*va_thresh*/
    0x52,

    /*uu_thresh*/
    0x40,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3

};


/*
 * This function initialises an instance of cmos_inttime_t.
 */
static cmos_inttime_const_ptr_t ar0130_inttime_initialize()
{
	static cmos_inttime_t cmos_inttime;
    //TODO: min/max integration time control.
   	//cmos_inttime.min_lines_std = 128;
	cmos_inttime.full_lines_std = 750;
	cmos_inttime.full_lines_std_30fps = 750;
	cmos_inttime.full_lines = 750;
	cmos_inttime.full_lines_del = 750; //TODO: remove
	cmos_inttime.full_lines_limit = 65535;
	cmos_inttime.max_lines = 748;
	cmos_inttime.min_lines = 2;
	cmos_inttime.vblanking_lines = 0;

	cmos_inttime.exposure_ashort = 0;
	cmos_inttime.exposure_shift = 0;

	cmos_inttime.lines_per_500ms = 750*30/2; // 500ms / 22.22us
	cmos_inttime.flicker_freq = 0;//60*256;//50*256;

	cmos_inttime.max_lines_target = cmos_inttime.max_lines;
	cmos_inttime.min_lines_target = cmos_inttime.min_lines;
	//cmos_inttime.max_flicker_lines = cmos_inttime.max_lines_target;
	//cmos_inttime.min_flicker_lines = cmos_inttime.min_lines_target;
	//cmos_inttime.input_changed = 0;

	return &cmos_inttime;
}

/*
 * This function applies the new integration time to the ISP registers.
 */
static void ar0130_inttime_update(cmos_inttime_ptr_t p_inttime) 
{
	HI_U16 _time = p_inttime->exposure_ashort >> p_inttime->exposure_shift;
	SENSOR_I2C_WRITE(0x3012, _time);
}

/*
 * This function applies the new vert blanking porch to the ISP registers.
 */
static void ar0130_vblanking_update(cmos_inttime_const_ptr_t p_inttime)
{
     int  _fulllines= p_inttime->full_lines;

       SENSOR_I2C_WRITE(0x300A, _fulllines);

}

static __inline HI_U16 ar0130_vblanking_calculate(
		cmos_inttime_ptr_t p_inttime)
{
	p_inttime->exposure_along  = p_inttime->exposure_ashort;

	if(p_inttime->exposure_along < p_inttime->full_lines_std - 2)
	{
		p_inttime->full_lines_del = p_inttime->full_lines_std;
	}
	if(p_inttime->exposure_along >= p_inttime->full_lines_std - 2)
	{
		p_inttime->full_lines_del = p_inttime->exposure_along + 2;
	}
#if defined(TRACE_ALL)
	alt_printf("full_lines_del = %x\n", p_inttime->full_lines_del);
#endif
	p_inttime->vblanking_lines = p_inttime->full_lines_del - 720;
#if defined(TRACE_ALL)
	alt_printf("vblanking_lines = %x\n", p_inttime->vblanking_lines);
#endif
	return p_inttime->exposure_ashort;
}

/* Set fps base */
static void ar0130_fps_set(
		cmos_inttime_ptr_t p_inttime,
		const HI_U8 fps
		)
{
	switch(fps)
	{
		case 30:
			p_inttime->full_lines_std = 750;
			p_inttime->lines_per_500ms = 750 * 30 / 2; 
			SENSOR_I2C_WRITE(0x300A, 0x2EE);
		break;
		
		case 25:
			p_inttime->full_lines_std = 900;
			p_inttime->lines_per_500ms = 900 * 25 / 2;
			SENSOR_I2C_WRITE(0x300A, 0x384);
		break;
		
		default:
		break;
	}
}

/*
 * This function initialises an instance of cmos_gains_t.
 */
static cmos_gains_ptr_t ar0130_gains_initialize()
{
	static cmos_gains_t cmos_gains;
	cmos_gains.max_again = 8;
	cmos_gains.max_dgain = 255;
	cmos_gains.max_again_target = cmos_gains.max_again;
	cmos_gains.max_dgain_target = cmos_gains.max_dgain;

	cmos_gains.again_shift = 0;
	cmos_gains.dgain_shift = 5;
	cmos_gains.dgain_fine_shift = 0;

	cmos_gains.again = 1;
	cmos_gains.dgain = 1;
	cmos_gains.dgain_fine = 1;
	cmos_gains.again_db = 0;
	cmos_gains.dgain_db = 0;

    cmos_gains.isp_dgain_shift = 4;
    cmos_gains.isp_dgain = 1 << cmos_gains.isp_dgain_shift;
    cmos_gains.max_isp_dgain_target = 1 << cmos_gains.isp_dgain_shift;

//	cmos_gains.input_changed = 0;

	return &cmos_gains;
}

/*
 * This function applies the new gains to the ISP registers.
 */
static void ar0130_gains_update(cmos_gains_const_ptr_t p_gains)
{
	int ag = p_gains->again; 
	int dg = p_gains->dgain;
	
	switch(ag)
	{
		case(0):
			SENSOR_I2C_WRITE(0x30B0, 0x1300);
			break;
		case(1):
			SENSOR_I2C_WRITE(0x30B0, 0x1300);
			break;
		case(2):
			SENSOR_I2C_WRITE(0x30B0, 0x1310);
			break;
		case(4):
			SENSOR_I2C_WRITE(0x30B0, 0x1320);
			break;
		case(8):
			SENSOR_I2C_WRITE(0x30B0, 0x1330);
			break;
	}
	
	SENSOR_I2C_WRITE(0x305E, dg);
}

/* Emulate digital fine gain */
static void ar0130_em_dgain_fine_update(cmos_gains_ptr_t p_gains)
{
}

static HI_U32 ar0130_gains_lin_to_db_convert(HI_U32 data, HI_U32 shift_in)
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

static HI_U32 ar0130_analog_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift)
{
	HI_U32 _again = 1<<p_gains->again_shift;
	//HI_U32 _ares = 1<<p_gains->again_shift;
	//HI_U32 _lres = 0;
	int shft = 0;

	while (exposure > (1<<20))
	{
		exposure >>= 1;
		exposure_max >>= 1;
		++shft;
	}

	if(exposure > exposure_max)
	{
#define DIV_0_TO_1(a)   ( (0 == a) ? 1 : a )
				//when setting manual exposure line, exposure_max>>shift should not be 0.
				exposure_max = DIV_0_TO_1(exposure_max);
		_again = (exposure	* _again)  / exposure_max;
//		exposure = exposure_max;

		if (_again >= 1<< 3) { _again = 1<<3; }
		else if (_again >= 1<< 2) { _again = 1<<2; }
		else if (_again >= 1<< 1) { _again = 1<<1; }
		else if (_again >= 1)	  { _again = 1;    }

		_again = _again < (1<<p_gains->again_shift) ? (1<<p_gains->again_shift) : _again;
		_again = _again > p_gains->max_again_target ? p_gains->max_again_target : _again;
		
		exposure = (exposure / _again);
	}
	else
	{
		//_again = (_again * exposure) / (exposure / exposure_shift) / exposure_shift;
	}

	p_gains->again = _again;
    p_gains->again_db = ar0130_gains_lin_to_db_convert(p_gains->again, p_gains->again_shift);
	return (exposure << shft);
}

static HI_U32 ar0130_digital_gain_from_exposure_calculate(
		cmos_gains_ptr_t p_gains,
		HI_U32 exposure,
		HI_U32 exposure_max,
		HI_U32 exposure_shift
		)
{
	HI_U32 _dgain = 1<<p_gains->dgain_shift;
	int shft = 0;

	while (exposure > (1<<20))
	{
		exposure >>= 1;
		exposure_max >>= 1;
		++shft;
	}

	if(exposure > exposure_max)
	{
		//when setting manual exposure line, exposure_max>>shift should not be 0.
			exposure_max = DIV_0_TO_1(exposure_max);
			_dgain = (exposure	* _dgain) / exposure_max;
			exposure = exposure_max;
	}
	else
	{
		//TODO: after anti-flick, dgain may need to decrease. 
		//_dgain = (_dgain * exposure) / (exposure / exposure_shift) / exposure_shift;
	}
	_dgain = _dgain < (1<<p_gains->dgain_shift) ? (1<<p_gains->dgain_shift) : _dgain;
	_dgain = _dgain > p_gains->max_dgain_target ? p_gains->max_dgain_target : _dgain;

	p_gains->dgain = _dgain;
    p_gains->dgain_db = ar0130_gains_lin_to_db_convert(p_gains->dgain, p_gains->dgain_shift);

	return exposure << shft;
}

static void ar0130_sensor_update(
	cmos_gains_const_ptr_t p_gains,
	cmos_inttime_ptr_t p_inttime,
	HI_U8 frame
    )
{
	if(frame == 0)
	{
		ar0130_inttime_update(p_inttime);
	}
	if(frame == 1)
	{
		ar0130_gains_update(p_gains);
	}    
}

static HI_U32 ar0130_cmos_get_ISO(cmos_gains_ptr_t p_gains)
{
	HI_U32 _again = p_gains->again == 0 ? 1 : p_gains->again;
	HI_U32 _dgain = p_gains->dgain == 0 ? 1 : p_gains->dgain;

	p_gains->iso =	((_again * _dgain * 100) >> (p_gains->again_shift + p_gains->dgain_shift));

	return p_gains->iso;
}

static HI_U8 ar0130_get_analog_gain(cmos_gains_ptr_t p_gains)
{
	//return cmos_gains_lin_to_db_convert(p_gains->again, p_gains->again_shift);
	return p_gains->again_db;
}

static HI_U8 ar0130_get_digital_gain(cmos_gains_ptr_t p_gains)
{
	//return cmos_gains_lin_to_db_convert(p_gains->dgain, p_gains->dgain_shift);
	return p_gains->dgain_db;
}

#if 0
static HI_U8 cmos_get_digital_fine_gain(cmos_gains_ptr_t p_gains)
{
    return ar0130_gains_lin_to_db_convert(p_gains->dgain_fine, p_gains->dgain_shift);
}
#endif

static HI_U32 ar0130_get_isp_default(cmos_isp_default_ptr_t p_coms_isp_default)
{
	if (NULL == p_coms_isp_default)
	{
		printf("null pointer when get isp default value!\n");
		return -1;
	}
	memcpy(p_coms_isp_default, &st_coms_isp_default, sizeof(cmos_isp_default_t));
	return 0;
}

static HI_U32 ar0130_get_isp_agc_table(cmos_isp_agc_table_ptr_t p_cmos_isp_agc_table)
{
	if (NULL == p_cmos_isp_agc_table)
	{
		printf("null pointer when get isp agc table value!\n");
		return -1;
	}
	memcpy(p_cmos_isp_agc_table, &st_isp_agc_table, sizeof(cmos_isp_agc_table_t));
	return 0;
}

static HI_U32 ar0130_get_isp_noise_table(cmos_isp_noise_table_ptr_t p_cmos_isp_noise_table)
{
	if (NULL == p_cmos_isp_noise_table)
	{
		printf("null pointer when get isp noise table value!\n");
		return -1;
	}
	memcpy(p_cmos_isp_noise_table, &st_isp_noise_table, sizeof(cmos_isp_noise_table_t));
	return 0;
}

static HI_U32 ar0130_get_isp_demosaic(cmos_isp_demosaic_ptr_t p_cmos_isp_demosaic)
{
   if (NULL == p_cmos_isp_demosaic)
   {
		printf("null pointer when get isp demosaic value!\n");
		return -1;
   }
   memcpy(p_cmos_isp_demosaic, &st_isp_demosaic,sizeof(cmos_isp_demosaic_t));
   return 0;

}
static void ar0130_setup_sensor(int isp_mode)
{
	if(0 == isp_mode) /* ISP 'normal' isp_mode */
	{
		SENSOR_I2C_WRITE(0x300C, 0xCE4);	//30fps
	}
	else if(1 == isp_mode) /* ISP pixel calibration isp_mode */
	{
		SENSOR_I2C_WRITE(0x300C, 0x4D58);	//5fps
		SENSOR_I2C_WRITE(0x3012, 0x05DA);	//max exposure lines
		SENSOR_I2C_WRITE(0x30B0, 0x1300);	//AG, Context A
		SENSOR_I2C_WRITE(0x305E, 0x0020);	//DG, Context A 
	}
}


void APTINA_AR0130_init(SENSOR_APTINA_AR0130_DO_I2CRD do_i2c_read, SENSOR_APTINA_AR0130_DO_I2CWR do_i2c_write)
{
	SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;


	// [720p @ 30fps]
	SENSOR_I2C_WRITE(0x301A, 0x0001);	// RESET_REGISTER
	SENSOR_I2C_WRITE(0x301A, 0x10D8);	// RESET_REGISTER
	
	SENSOR_DELAY_MS(200);	//DELAY= 200
	
	SENSOR_I2C_WRITE(0x3088, 0x8000);	// SEQ_CTRL_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0225);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x5050);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2D26);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0828);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0D17);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0926);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0028);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0526);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0xA728);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0725);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x8080);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2917);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0525);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0040);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2702);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1616);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2706);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1736);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x26A6);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1703);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x26A4);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x171F);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2805);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2620);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2804);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2520);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2027);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0017);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1E25);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0020);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2117);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1028);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x051B);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1703);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2706);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1703);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1741);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2660);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x17AE);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2500);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x9027);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0026);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1828);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x002E);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2A28);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x081E);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0831);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1440);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x4014);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2020);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1410);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1034);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1400);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1014);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0020);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1400);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x4013);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1802);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1470);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x7004);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1470);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x7003);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1470);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x7017);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2002);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1400);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2002);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1400);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x5004);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1400);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2004);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x1400);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x5022);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0314);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0020);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0314);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x0050);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2C2C);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x3086, 0x2C2C);	// SEQ_DATA_PORT
	SENSOR_I2C_WRITE(0x309E, 0x0000);	// ERS_PROG_START_ADDR
	
	SENSOR_DELAY_MS(200);	//DELAY= 200
	
	SENSOR_I2C_WRITE(0x30E4, 0x6372);	// ADC_BITS_6_7
	SENSOR_I2C_WRITE(0x30E2, 0x7253);	// ADC_BITS_4_5
	SENSOR_I2C_WRITE(0x30E0, 0x5470);	// ADC_BITS_2_3
	SENSOR_I2C_WRITE(0x30E6, 0xC4CC);	// ADC_CONFIG1
	SENSOR_I2C_WRITE(0x30E8, 0x8050);	// ADC_CONFIG2
	SENSOR_I2C_WRITE(0x3082, 0x0029);	// OPERATION_MODE_CTRL
	SENSOR_I2C_WRITE(0x30B0, 0x1300);	// DIGITAL_TEST
	SENSOR_I2C_WRITE(0x30D4, 0xE007);	// COLUMN_CORRECTION
	SENSOR_I2C_WRITE(0x301A, 0x10DC);	// RESET_REGISTER
	SENSOR_I2C_WRITE(0x301A, 0x10D8);	// RESET_REGISTER
	SENSOR_I2C_WRITE(0x3044, 0x0400);	// DARK_CONTROL
	SENSOR_I2C_WRITE(0x3EDA, 0x0F03);	// DAC_LD_14_15
	SENSOR_I2C_WRITE(0x3ED8, 0x01EF);	// DAC_LD_12_13
	SENSOR_I2C_WRITE(0x3012, 0x02A0);	// COARSE_INTEGRATION_TIME
	SENSOR_I2C_WRITE(0x3032, 0x0000);	// DIGITAL_BINNING
	SENSOR_I2C_WRITE(0x3002, 0x003e);	// Y_ADDR_START
	SENSOR_I2C_WRITE(0x3004, 0x0004);	// X_ADDR_START
	SENSOR_I2C_WRITE(0x3006, 0x030d);	// Y_ADDR_END
	SENSOR_I2C_WRITE(0x3008, 0x0503);	// X_ADDR_END
	SENSOR_I2C_WRITE(0x300A, 0x02EE);	// FRAME_LENGTH_LINES
	SENSOR_I2C_WRITE(0x300C, 0x0CE4);	// LINE_LENGTH_PCK
	SENSOR_I2C_WRITE(0x301A, 0x10D8);	// RESET_REGISTER
	SENSOR_I2C_WRITE(0x31D0, 0x0001);	// HDR_COMP

	//Load = PLL Enabled 27Mhz to 74.25Mhz
	SENSOR_I2C_WRITE(0x302C, 0x0002);	// VT_SYS_CLK_DIV
	SENSOR_I2C_WRITE(0x302A, 0x0004);	// VT_PIX_CLK_DIV
	SENSOR_I2C_WRITE(0x302E, 0x0002);	// PRE_PLL_CLK_DIV
	SENSOR_I2C_WRITE(0x3030, 0x002C);	// PLL_MULTIPLIER
	SENSOR_I2C_WRITE(0x30B0, 0x0000);	// DIGITAL_TEST 
	SENSOR_DELAY_MS(100);	//DELAY= 100

	//LOAD= Disable Embedded Data and Stats
	SENSOR_I2C_WRITE(0x3064, 0x1802);	// SMIA_TEST, EMBEDDED_STATS_EN, 0x0000
	SENSOR_I2C_WRITE(0x3064, 0x1802);	// SMIA_TEST, EMBEDDED_DATA, 0x0000 

	SENSOR_I2C_WRITE(0x30BA, 0x0008);		 //20120502

	SENSOR_I2C_WRITE(0x301A, 0x10DC);	// RESET_REGISTER

	SENSOR_DELAY_MS(200);	//DELAY= 200
	memset(&sensor_exp_func, 0, sizeof(sensor_exp_func));	
	sensor_exp_func.pfn_cmos_inttime_initialize = ar0130_inttime_initialize;
	sensor_exp_func.pfn_cmos_inttime_update = ar0130_inttime_update;
	sensor_exp_func.pfn_cmos_gains_initialize = ar0130_gains_initialize;
	sensor_exp_func.pfn_cmos_gains_update = ar0130_gains_update;
	sensor_exp_func.pfn_cmos_gains_update2 = NULL;
	sensor_exp_func.pfn_analog_gain_from_exposure_calculate = ar0130_analog_gain_from_exposure_calculate;
	sensor_exp_func.pfn_digital_gain_from_exposure_calculate = ar0130_digital_gain_from_exposure_calculate;
	sensor_exp_func.pfn_cmos_fps_set = ar0130_fps_set;
	sensor_exp_func.pfn_vblanking_calculate = ar0130_vblanking_calculate;
	sensor_exp_func.pfn_cmos_vblanking_front_update = ar0130_vblanking_update;
	sensor_exp_func.pfn_setup_sensor = ar0130_setup_sensor;
	sensor_exp_func.pfn_cmos_get_analog_gain = ar0130_get_analog_gain;
	sensor_exp_func.pfn_cmos_get_digital_gain = ar0130_get_digital_gain;
	sensor_exp_func.pfn_cmos_get_digital_fine_gain = NULL;
	sensor_exp_func.pfn_cmos_get_iso = ar0130_cmos_get_ISO;
	sensor_exp_func.pfn_cmos_get_isp_default = ar0130_get_isp_default;
	sensor_exp_func.pfn_cmos_get_isp_special_alg = NULL;
	sensor_exp_func.pfn_cmos_get_isp_agc_table = ar0130_get_isp_agc_table;
	sensor_exp_func.pfn_cmos_get_isp_noise_table = ar0130_get_isp_noise_table;
	sensor_exp_func.pfn_cmos_get_isp_demosaic = ar0130_get_isp_demosaic;
	sensor_exp_func.pfn_cmos_get_isp_shading_table = NULL;
	SOC_CHECK(HI_MPI_ISP_SensorRegCallBack(&sensor_exp_func));

	printf("Aptina AR0130 sensor 720P30fps init success!\n");
		
}


