/*   extdrv/peripheral/dc/mt9v131.c
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd.
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
 * along with this program.
 *
 *
 * History:
 *     04-Apr-2006 create this file
 *
 */

#include "mt9d131.h"
#include "sensor.h"
#include "sysconf.h"

// mt9d131 i2c slaver address micro-definition
#define MT9D131_I2C_DEV (0x90)
#define MT9D131_MSLEEP(ms) do{usleep((ms) << 10);}while(0)

static MT9D131_WRITE_FUNC _mt9d131_w = NULL;
static MT9D131_READ_FUNC _mt9d131_r = NULL;

static int mt9d131_write(uint8_t addr, uint8_t val)
{
	if(_mt9d131_w){
		return _mt9d131_w(MT9D131_I2C_DEV, addr, val);
	}
	return -1;
}

static uint8_t mt9d131_read(uint8_t addr)
{
	if(_mt9d131_r){
		return _mt9d131_r(MT9D131_I2C_DEV, addr);
	}
	return 0;
}

static void mt9d131_common_init(int freq)
{
	/*========soft reset===============*/
	/* page 0 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00);    //ser page 0

	mt9d131_write(0x65, 0xa0);
	mt9d131_write(0xf1, 0x00); 
	/* page 1 */
	mt9d131_write(0xf0, 0x00);   //ser page 1
	mt9d131_write(0xf1, 0x01);    

	mt9d131_write(0xc3, 0x05);
	mt9d131_write(0xf1, 0x01);  
	/* page 0 */
	mt9d131_write(0xf0, 0x00);   //ser page 0
	mt9d131_write(0xf1, 0x00); 

	mt9d131_write(0x0d, 0x00);
	mt9d131_write(0xf1, 0x21);   

	MT9D131_MSLEEP(10);  

	mt9d131_write(0x0d, 0x00);
	mt9d131_write(0xf1, 0x00);   

	MT9D131_MSLEEP(10); 
	/* soft reset end */ 

	/* pll control */
	mt9d131_write(0x66, 0x10);
	mt9d131_write(0xf1, 0x04);

	mt9d131_write(0x67, 0x05);
	mt9d131_write(0xf1, 0x00); 

	MT9D131_MSLEEP(50); 

	mt9d131_write(0x65, 0xA0);
	mt9d131_write(0xf1, 0x00);

	MT9D131_MSLEEP(50);      

	mt9d131_write(0x65, 0x20);
	mt9d131_write(0xf1, 0x00);   

	MT9D131_MSLEEP(100);      
	/* pll control end */

	/**************set mode*******************************************/   
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);      //ser page 1
	/* contexa/b  bypass jpeg */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x0B);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x30);

	/* page 0 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00);    //ser page 0
	/* Read Mode (A) */ 
	mt9d131_write(0x21, 0x03);    
	mt9d131_write(0xf1, 0x00);


	/************set mode end******************************************/    

	/************************flicker detection****************************************/
	mt9d131_write(0xf0, 0x00); 
	mt9d131_write(0xf1, 0x01);    //ser page 1

	/*  search_f1_50  Lower limit of period range  30 */       
	mt9d131_write(0xC6, 0xA4);        
	mt9d131_write(0xf1, 0x08);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x24);
	/*   search_f1_50  upper limit of period range  32 */ 
	mt9d131_write(0xC6, 0xA4);        
	mt9d131_write(0xf1, 0x09);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x26);
	/*  search_f1_60  Lower limit of period range  37  */   
	mt9d131_write(0xC6, 0xA4);        
	mt9d131_write(0xf1, 0x0a);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x1e);
	/*  search_f1_60  upper limit of period range  39  */
	mt9d131_write(0xC6, 0xA4);        
	mt9d131_write(0xf1, 0x0b);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x20);
	/* R9_Step_60   minimal shutter width step for 60hz ac  157 */
	mt9d131_write(0xC6, 0x24);        
	mt9d131_write(0xf1, 0x11);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x9c);
	/* R9_Step_50   minimal shutter width step for 50hz ac  188 */
	mt9d131_write(0xC6, 0x24);        
	mt9d131_write(0xf1, 0x13);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0xbc);

	if(50 == freq)
	{   
		// fix to 50hz
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01);   //ser page 1

		mt9d131_write(0xC6, 0xa1);        
		mt9d131_write(0xf1, 0x2a);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x02);

		mt9d131_write(0xC6, 0xa4);        
		mt9d131_write(0xf1, 0x04);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0xc0);

		mt9d131_write(0xC6, 0xa1);        
		mt9d131_write(0xf1, 0x03);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x06);
	}else{
		// fix to 60hz
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01);   //ser page 1

		mt9d131_write(0xC6, 0xa1);        
		mt9d131_write(0xf1, 0x2a);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x02);

		mt9d131_write(0xC6, 0xa4);        
		mt9d131_write(0xf1, 0x04);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x80);

		mt9d131_write(0xC6, 0xa1);        
		mt9d131_write(0xf1, 0x03);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x06);
	}
	/************************************flicker detection  end ****************************/ 
	/***************************************************set auto exposure*********/
	mt9d131_write(0xf0, 0x00); 
	mt9d131_write(0xf1, 0x01);    //ser page 1
	/* Max R12 (B)(Shutter Delay)  402 */
	mt9d131_write(0xC6, 0x22);        
	mt9d131_write(0xf1, 0x0b);
	mt9d131_write(0xC8, 0x01);        
	mt9d131_write(0xf1, 0x92);
	/* IndexTH23  Zone number to start gain increase in low-light. 
	Sets  frame rate at normal illumination.   4 */
	mt9d131_write(0xC6, 0xA2);        
	mt9d131_write(0xf1, 0x17);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x04);
	/* RowTime (msclk per)/4  Row time divided by 4 (in master clock periods)  527 */
	mt9d131_write(0xC6, 0x22);        
	mt9d131_write(0xf1, 0x28);
	mt9d131_write(0xC8, 0x02);        
	mt9d131_write(0xf1, 0x0f);
	/* R9 Step   Integration time of one zone  156 */
	mt9d131_write(0xC6, 0x22);        
	mt9d131_write(0xf1, 0x2f);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x9c);
	/* Maximum allowed zone number (that is maximumintegration time)  4 */
	mt9d131_write(0xC6, 0xa2);        
	mt9d131_write(0xf1, 0x0e);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x04);
	
	mt9d131_write(0xC6, 0x22);        
	mt9d131_write(0xf1, 0x14);
	mt9d131_write(0xC8, 0x01);        
	mt9d131_write(0xf1, 0xff);

	/***************************************************set auto exposure  end*********/    
	/*======******************************************lens correcton***************************/
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x02);   //ser page 2

	mt9d131_write(0x80, 0x01);
	mt9d131_write(0xf1, 0xE8); 	/* LENS_CORRECTION_CONTROL */

	mt9d131_write(0x81, 0x5C);
	mt9d131_write(0xf1, 0x2E); 	/* ZONE_BOUNDS_X1_X2 */

	mt9d131_write(0x82, 0x36);
	mt9d131_write(0xf1, 0x8A); 	/* ZONE_BOUNDS_X0_X3 */

	mt9d131_write(0x83, 0xA2);
	mt9d131_write(0xf1, 0x6C); 	/* ZONE_BOUNDS_X4_X5 */   

	mt9d131_write(0x84, 0x4B);
	mt9d131_write(0xf1, 0x26); 	/* ZONE_BOUNDS_Y1_Y2 */

	mt9d131_write(0x85, 0x25);
	mt9d131_write(0xf1, 0x71); 	/* ZONE_BOUNDS_Y0_Y3  */ 

	mt9d131_write(0x86, 0x70);
	mt9d131_write(0xf1, 0x4B); 	/* ZONE_BOUNDS_Y4_Y5  */

	mt9d131_write(0x87, 0x01);
	mt9d131_write(0xf1, 0xF0); 	/* CENTER_OFFSET */

	mt9d131_write(0x88, 0x00);
	mt9d131_write(0xf1, 0x02); 	/* FX_RED */

	mt9d131_write(0x8B, 0x00);
	mt9d131_write(0xf1, 0x20); 	/* FY_RED */

	mt9d131_write(0x8E, 0x03);
	mt9d131_write(0xf1, 0xE2); 	/* DF_DX_RED */

	mt9d131_write(0x91, 0x00);
	mt9d131_write(0xf1, 0x20); 	/* DF_DY_RED */

	mt9d131_write(0x94, 0xC4);
	mt9d131_write(0xf1, 0xB8); 	/* SECOND_DERIV_ZONE_0_RED */

	mt9d131_write(0x97, 0x25);
	mt9d131_write(0xf1, 0x8E); 	/* SECOND_DERIV_ZONE_1_RED */

	mt9d131_write(0x9A, 0x03);
	mt9d131_write(0xf1, 0xEB); 	/* SECOND_DERIV_ZONE_2_RED */

	mt9d131_write(0x9D, 0x26);
	mt9d131_write(0xf1, 0x2C); 	/* SECOND_DERIV_ZONE_3_RED */

	mt9d131_write(0xA0, 0x2C);
	mt9d131_write(0xf1, 0x05); 	/* SECOND_DERIV_ZONE_4_RED */

	mt9d131_write(0xA3, 0x09);
	mt9d131_write(0xf1, 0x25); 	/* SECOND_DERIV_ZONE_5_RED */

	mt9d131_write(0xA6, 0xEE);
	mt9d131_write(0xf1, 0xEB); 	/* SECOND_DERIV_ZONE_6_RED */

	mt9d131_write(0xA9, 0xD9);
	mt9d131_write(0xf1, 0xB4); 	/* SECOND_DERIV_ZONE_7_RED */

	mt9d131_write(0x89, 0x00);
	mt9d131_write(0xf1, 0x02); 	/* FX_GREEN */

	mt9d131_write(0x8C, 0x00);
	mt9d131_write(0xf1, 0x1B); 	/* FY_GREEN */

	mt9d131_write(0x8F, 0x04);
	mt9d131_write(0xf1, 0x29); 	/* DF_DX_GREEN */

	mt9d131_write(0x92, 0x01);
	mt9d131_write(0xf1, 0xF5); 	/* DF_DY_GREEN */

	mt9d131_write(0x95, 0xAE);
	mt9d131_write(0xf1, 0xAE); 	/* SECOND_DERIV_ZONE_0_GREEN */  

	mt9d131_write(0x98, 0xFA);
	mt9d131_write(0xf1, 0x9C); 	/* SECOND_DERIV_ZONE_1_GREEN */

	mt9d131_write(0x9B, 0x05);
	mt9d131_write(0xf1, 0xE3); 	/* SECOND_DERIV_ZONE_2_GREEN */

	mt9d131_write(0x9E, 0x17);
	mt9d131_write(0xf1, 0x2F); 	/* SECOND_DERIV_ZONE_3_GREEN */

	mt9d131_write(0xA1, 0x2A);
	mt9d131_write(0xf1, 0x00); 	/* SECOND_DERIV_ZONE_4_GREEN */

	mt9d131_write(0xA4, 0x14);
	mt9d131_write(0xf1, 0x23); 	/* SECOND_DERIV_ZONE_5_GREEN */

	mt9d131_write(0xA7, 0xE8);
	mt9d131_write(0xf1, 0xDE); 	/* SECOND_DERIV_ZONE_6_GREEN */

	mt9d131_write(0xAA, 0xAF);
	mt9d131_write(0xf1, 0xAA); 	/* SECOND_DERIV_ZONE_7_GREEN */

	mt9d131_write(0x8A, 0x00);
	mt9d131_write(0xf1, 0x02); 	/* FX_BLUE */

	mt9d131_write(0x8D, 0x00);
	mt9d131_write(0xf1, 0x0E); 	/* FY_BLUE  */ 

	mt9d131_write(0x90, 0x04);
	mt9d131_write(0xf1, 0x6B); 	/* DF_DX_BLUE */

	mt9d131_write(0x93, 0x01);
	mt9d131_write(0xf1, 0x92); 	/* DF_DY_BLUE */

	mt9d131_write(0x96, 0xBC);
	mt9d131_write(0xf1, 0xBF); 	/* SECOND_DERIV_ZONE_0_BLUE */

	mt9d131_write(0x99, 0x0B);
	mt9d131_write(0xf1, 0x8E); 	/* SECOND_DERIV_ZONE_1_BLUE */

	mt9d131_write(0x9C, 0xFF);
	mt9d131_write(0xf1, 0xDB); 	/* SECOND_DERIV_ZONE_2_BLUE */

	mt9d131_write(0x9F, 0x11);
	mt9d131_write(0xf1, 0x29); 	/* SECOND_DERIV_ZONE_3_BLUE */

	mt9d131_write(0xA2, 0x24);
	mt9d131_write(0xf1, 0xF2); 	/* SECOND_DERIV_ZONE_4_BLUE  */ 

	mt9d131_write(0xA5, 0x10);
	mt9d131_write(0xf1, 0x23); 	/* SECOND_DERIV_ZONE_5_BLUE */

	mt9d131_write(0xA8, 0xC9);
	mt9d131_write(0xf1, 0xE2); 	/* SECOND_DERIV_ZONE_6_BLUE */

	mt9d131_write(0xAB, 0xCA);
	mt9d131_write(0xf1, 0xB0); 	/* SECOND_DERIV_ZONE_7_BLUE */

	mt9d131_write(0xAC, 0x80);
	mt9d131_write(0xf1, 0x00); 	/* X2_FACTORS */

	mt9d131_write(0xAD, 0x00);
	mt9d131_write(0xf1, 0x00); 	/* GLOBAL_OFFSET_FXY_FUNCTION */

	mt9d131_write(0xAE, 0x01);
	mt9d131_write(0xf1, 0x8E); 	/* K_FACTOR_IN_K_FX_FY */

	mt9d131_write(0x08, 0x01);
	mt9d131_write(0xf1, 0xFC); 	/* COLOR_PIPELINE_CONTROL */

	/* page 1 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);   //ser page 1
	/* enable  lc/gamma/color corretion */
	mt9d131_write(0x08, 0x01);    
	mt9d131_write(0xf1, 0xFC);
	/* Refresh Sequencer Mode  6 */
	mt9d131_write(0xC6, 0xa1);
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x06); 
	/* refresh sequencer 5 */
	mt9d131_write(0xC6, 0xa1);
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x05);       
	/*********************************************lens correction end***********/

	/********************************gamma and contrast****************************/

	/* page 1 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);   //ser page 1

	mt9d131_write(0xC6, 0xA7);
	mt9d131_write(0xf1, 0x43);  /* MCU_ADDRESS [MODE_GAM_CONT_A] */
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x12);  /* MCU_DATA_0  */  


	mt9d131_write(0xC6, 0xA1);
	mt9d131_write(0xf1, 0x03);  /* MCU_ADDRESS [SEQ_CMD] */
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x05);  /* MCU_DATA_0 */
	/**************************************gamma and contrast end***************************/

	/*--------use preview mode, not capture mode--------*/
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);   //ser page 1
	/* capture  clear */
	mt9d131_write(0xc6, 0xa1);
	mt9d131_write(0xf1, 0x20); 
	mt9d131_write(0xc8, 0x00);
	mt9d131_write(0xf1, 0x00);
	/* cmd do preview */
	mt9d131_write(0xc6, 0xa1);
	mt9d131_write(0xf1, 0x03); 
	mt9d131_write(0xc8, 0x00);
	mt9d131_write(0xf1, 0x01);

}

static void mt9d131_vga_init(int freq)
{
	/************************************************set mode*********************************/
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00);
	/* HBLANK (A) = 174 */
	mt9d131_write(0x07, 0x00);    
	mt9d131_write(0xf1, 0xae);
	/* VBLANK (A) = 16 */
	mt9d131_write(0x08, 0x00);   
	mt9d131_write(0xf1, 0x10);

	/* page 1 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);   

	/* output_width 800 a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x03);
	mt9d131_write(0xf1, 0x20);
	/* output_height 600 a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x05);
	mt9d131_write(0xC8, 0x02);
	mt9d131_write(0xf1, 0x58);
	/* first sensor-readout row 28  context a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x0F);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x1C);
	/* first sensor-readout column 60 context a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x11);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x3C);
	/* contexta number of sensor-readout rows 600 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x13);
	mt9d131_write(0xC8, 0x02);
	mt9d131_write(0xf1, 0x58);
	/* contexta number of sensor-readout columns 800 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x15);
	mt9d131_write(0xC8, 0x03);
	mt9d131_write(0xf1, 0x20);

	/* extra sensor delay per frame context a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x17);
	mt9d131_write(0xC8, 0x03);
	mt9d131_write(0xf1, 0x18);
	/* row-speed context a 17 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x19);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x11);
	/* Crop_X0 (A)  0 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x27);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x00);
	/* Crop_X1 (A)  800 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x29);
	mt9d131_write(0xC8, 0x03);        
	mt9d131_write(0xf1, 0x20);
	/* Crop_Y0 (A)  0 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x2b);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x00);
	/* Crop_Y1 (A)  600 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x2d);
	mt9d131_write(0xC8, 0x02);        
	mt9d131_write(0xf1, 0x58);
	/* FIFO_Conf1 (A)   57570 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x6d);
	mt9d131_write(0xC8, 0xE0);       
	mt9d131_write(0xf1, 0xe2);
	/* FIFO_Conf2 (A)   225 */
	mt9d131_write(0xC6, 0xA7);        
	mt9d131_write(0xf1, 0x6f);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0xe1);
	/***************************************************set mode end*************/

	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01); 

	MT9D131_MSLEEP(500);     /* DELAY = 500 */

	mt9d131_write(0xC6, 0xA1);    /* Refresh Sequencer Mode */
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);    /*  = 6 */
	mt9d131_write(0xf1, 0x06);

	MT9D131_MSLEEP(500);     /* DELAY = 500 */

	mt9d131_write(0xC6, 0xA1);    /* Refresh Sequencer */
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);    /*  = 5 */
	mt9d131_write(0xf1, 0x05);

}

static void mt9d131_xxga_init(int freq)
{
	mt9d131_common_init(freq);
	/************************************************set mode*********************************/
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00);   //ser page 0
	/* HBLANK (A) = 286 */
	mt9d131_write(0x07, 0x01);    
	mt9d131_write(0xf1, 0x1e);/* 1280x720 */

	/* VBLANK (A) = 119 */
	mt9d131_write(0x08, 0x00);          
	//mt9d131_write(0xf1, 0x0b);/* 33fps */
	mt9d131_write(0xf1, 0x77);/* 30fps */
	/* page 1 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);      //ser page 1

	/* output_width 1600 a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x05);
	mt9d131_write(0xf1, 0x00);

	/* output_height 1200 a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x05);
	mt9d131_write(0xC8, 0x02);
	mt9d131_write(0xf1, 0xd0);

	/* first sensor-readout row 28  context a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x0F);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x1C);
	/* first sensor-readout column 60 context a */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x11);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x3C);
	/* contexta number of sensor-readout rows 720 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x13);
	mt9d131_write(0xC8, 0x02);
	mt9d131_write(0xf1, 0xd0);
	/* contexta number of sensor-readout columns 1280 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x15);
	mt9d131_write(0xC8, 0x05);
	mt9d131_write(0xf1, 0x00);


	/* extra sensor delay per frame context a 155 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x17);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x9b);
	/* row-speed context a 17 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x19);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x11);
	/* Crop_X0 (A)  0 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x27);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x00);
	/* Crop_X1 (A)  1600 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x29);
	mt9d131_write(0xC8, 0x05);
	mt9d131_write(0xf1, 0x00);
	/* Crop_Y0 (A)  0 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x2b);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x00);
	/* Crop_Y1 (A)  1200 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x2d);
	mt9d131_write(0xC8, 0x02);
	mt9d131_write(0xf1, 0xd0);
	/* FIFO_Conf1 (A)   57569 */
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x6d);
	mt9d131_write(0xC8, 0xE0);       
	mt9d131_write(0xf1, 0xe1);


	mt9d131_write(0xC6, 0xA7);        
	mt9d131_write(0xf1, 0x6f);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0xe1);
	/***************************************************set mode end*************/

	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);    //ser page 1

	MT9D131_MSLEEP(500);     /* DELAY = 500 */

	mt9d131_write(0xC6, 0xA1);    /* Refresh Sequencer Mode */
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);    /*  = 6 */
	mt9d131_write(0xf1, 0x06);

	MT9D131_MSLEEP(500);     /* DELAY = 500 */

	mt9d131_write(0xC6, 0xA1);    /* Refresh Sequencer */
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);    /*  = 5 */
	mt9d131_write(0xf1, 0x05);
}


static void mt9d131_uxga_init(int freq)
{
	mt9d131_common_init(freq);
	/************************************************set mode*********************************/
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00);
	/* HBLANK (A) = 516 */
	mt9d131_write(0x07, 0x02);    
	mt9d131_write(0xf1, 0x04);
	/* VBLANK (A) = 47 */
	mt9d131_write(0x08, 0x00);   
	mt9d131_write(0xf1, 0x2f);

	/* page 1 */
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01);   

	/*output_width 1600 a*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x06);
	mt9d131_write(0xf1, 0x40);
	/*output_height 1200 a*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x05);
	mt9d131_write(0xC8, 0x04);
	mt9d131_write(0xf1, 0xb0);
	/*first sensor-readout row 28  context a*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x0F);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x1C);
	/*first sensor-readout column 60 context a*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x11);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x3C);
	/*contexta number of sensor-readout rows 1200 */
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x13);
	mt9d131_write(0xC8, 0x04);
	mt9d131_write(0xf1, 0xb0);
	/*contexta number of sensor-readout columns 1600*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x15);
	mt9d131_write(0xC8, 0x06);
	mt9d131_write(0xf1, 0x40);

	/*extra sensor delay per frame context a 1046*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x17);
	mt9d131_write(0xC8, 0x04);
	mt9d131_write(0xf1, 0x16);
	/*row-speed context a 17*/
	mt9d131_write(0xC6, 0x27);
	mt9d131_write(0xf1, 0x19);
	mt9d131_write(0xC8, 0x00);
	mt9d131_write(0xf1, 0x11);
	/*Crop_X0 (A)  0*/
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x27);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x00);
	/*Crop_X1 (A)  1600*/
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x29);
	mt9d131_write(0xC8, 0x06);        
	mt9d131_write(0xf1, 0x40);
	/*Crop_Y0 (A)  0*/
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x2b);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0x00);
	/*Crop_Y1 (A)  1200*/
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x2d);
	mt9d131_write(0xC8, 0x04);        
	mt9d131_write(0xf1, 0xb0);
	/*FIFO_Conf1 (A)   57569*/
	mt9d131_write(0xC6, 0x27);        
	mt9d131_write(0xf1, 0x6d);
	mt9d131_write(0xC8, 0xE0);       
	mt9d131_write(0xf1, 0xe1);
	/*FIFO_Conf2 (A)   225*/
	mt9d131_write(0xC6, 0xA7);        
	mt9d131_write(0xf1, 0x6f);
	mt9d131_write(0xC8, 0x00);        
	mt9d131_write(0xf1, 0xe1);
	/***************************************************set mode end*************/

	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x01); 

	MT9D131_MSLEEP(500);     /* DELAY = 500 */

	mt9d131_write(0xC6, 0xA1);    /* Refresh Sequencer Mode */
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);    /*  = 6 */
	mt9d131_write(0xf1, 0x06);

	MT9D131_MSLEEP(500);     /* DELAY = 500 */

	mt9d131_write(0xC6, 0xA1);    /* Refresh Sequencer */
	mt9d131_write(0xf1, 0x03);
	mt9d131_write(0xC8, 0x00);    /*  = 5 */
	mt9d131_write(0xf1, 0x05);
}

int MT9D131_install(MT9D131_READ_FUNC rfunc, MT9D131_WRITE_FUNC wfunc)
{
	if(!wfunc || !rfunc){
		return -1;
	}
	// install r/w function
	_mt9d131_r = rfunc;
	_mt9d131_w = wfunc;
	return 0;
}

int MT9D131_check()
{
	uint8_t reg = 0;
	uint8_t loop = 0;
	// read chip version
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00);

	reg = mt9d131_read(0x00);
	loop = mt9d131_read(0xf1); 
	if((reg != 0x15) || (loop != 0x19)){
		printf("read Prodect ID Number MSB is %x\n", reg);
		printf("read Prodect ID Number LSB is %x\n", loop);
		printf("check mt9d131 ID error.\n");
		return -1;
	}
	return 0;
}

int MT9D131_init(int freq)
{
	mt9d131_xxga_init(freq);
	return 0;
}

void MT9D131_mirror_flip(unsigned char mode)
{
	uint8_t high_byte = 0;
	uint8_t low_byte = 0;
	SYSCONF_t* sysconf = SYSCONF_dup();
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x00); //set page 0
	switch(mode){
		case MODE_MIRROR:
		{
			high_byte = mt9d131_read(0x20);
			low_byte = mt9d131_read(0xf1); 
			low_byte |=0x02;
			mt9d131_write(0x20, high_byte);
			mt9d131_write(0xf1, low_byte);
			printf("sensor mirror\r\n");
		}
		break;
		case MODE_FLIP:
		{
			high_byte = mt9d131_read(0x20);
			low_byte = mt9d131_read(0xf1); 
			low_byte |=0x01;
			mt9d131_write(0x20, high_byte);
			mt9d131_write(0xf1, low_byte);
			printf("sensor flip\r\n");
		}
		break;
		case MODE_UNMIRROR:
		{
			high_byte = mt9d131_read(0x20);
			low_byte = mt9d131_read(0xf1); 
			low_byte &=~(0x02);
			mt9d131_write(0x20, high_byte);
			mt9d131_write(0xf1, low_byte);
			printf("sensor unmirror\r\n");
		}
		break;
		case MODE_UNFLIP:
		{
			high_byte = mt9d131_read(0x20);
			low_byte = mt9d131_read(0xf1); 
			low_byte &=~(0x01);
			mt9d131_write(0x20, high_byte);
			mt9d131_write(0xf1, low_byte);
			printf("sensor unflip\r\n");
		}
		break;
		case MODE_NORMAL:
		{
			high_byte = mt9d131_read(0x20);
			low_byte = mt9d131_read(0xf1); 
			low_byte &=~(0x03);
			mt9d131_write(0x20, high_byte);
			mt9d131_write(0xf1, low_byte);
			printf("sensor unflip\r\n");
		}
		break;
		case MODE_MIRROR_FLIP:
		{
			high_byte = mt9d131_read(0x20);
			low_byte = mt9d131_read(0xf1); 
			low_byte |= 0x03;
			mt9d131_write(0x20, high_byte);
			mt9d131_write(0xf1, low_byte);
			printf("sensor unflip\r\n");
		}
		break;
		default:
			break;
	}
}
void MT9D131_test_mode(unsigned char enable){}
void MT9D131_AWB(unsigned char mode){}
void MT9D131_light_mode(unsigned char mode){}
void MT9D131_set_hue(unsigned short val)
{
	printf("\r\n");
}

void MT9D131_set_saturation(unsigned char val)//val should be 0,1,2,3,4,5,6,7
{
	uint8_t reg = 0;
	uint8_t loop = 0;

	ColorMaxValue max_value = MT9D131_get_color_max_value();
	/*mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x02); //set page 2
	
	reg = mt9d131_read(0xd2);
	loop = mt9d131_read(0xf1); 
	loop &= 0xf8; //b0~b2
	loop |= (val & 0x07);
	printf("saturation changed %d\r\n", val);
	mt9d131_write(0xd2, reg);
	mt9d131_write(0xf1, loop);*/
	if(val <= max_value.SaturationMax){
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01); //set page 1

		loop = val;
		printf("saturation changed %d\r\n", val);
		mt9d131_write(0xC6, 0xA3);
		mt9d131_write(0xf1, 0x52); 
		mt9d131_write(0xC8, 0x00);
		mt9d131_write(0xf1, loop); 
	}
	else{
		printf("saturation set:invalid value %d/%d\r\n", val, max_value.SaturationMax);
	}
}
unsigned char  MT9D131_get_saturation()
{
	uint8_t reg = 0;
	uint8_t loop = 0;

	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, 0x02); //set page 2

	reg = mt9d131_read(0xd2);
	loop = mt9d131_read(0xf1); 
	loop &= 0x07; //b0~b2

	return loop;
}
void MT9D131_set_brightness(unsigned char val)
{
	uint8_t reg = 0;
	uint8_t loop = 0;

	ColorMaxValue max_value = MT9D131_get_color_max_value();
	if(val < max_value.BrightnessMax){
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01); //set page 1

		loop = val;
		if(loop < 10){
			loop = 10;
		}else if(loop > 210){
			loop = 210;
		}
		printf("brightness changed %d\r\n", loop);
		mt9d131_write(0xC6, 0xA2);
		mt9d131_write(0xf1, 0x06); 
		mt9d131_write(0xC8, 0x00);
		mt9d131_write(0xf1, loop);   
	}
	else{
		printf("brightness set:invalid value %d/%d\r\n", val, max_value.BrightnessMax);
	}
}
void MT9D131_set_contrast(unsigned char val)
{
	uint8_t reg = 0;
	uint8_t loop = 0;
	
	ColorMaxValue max_value = MT9D131_get_color_max_value();
	if(val < max_value.BrightnessMax){
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01); //set page 1

		//loop = (1<<(val&0x07))|(1<<((val&0x70)>>4));
		loop = (val<<4)|(1<<1);
		printf("contrast changed %02x\r\n", val);
		mt9d131_write(0xC6, 0xA7);
		mt9d131_write(0xf1, 0x43);  /* MCU_ADDRESS [MODE_GAM_CONT_A] */
		mt9d131_write(0xC8, 0x00);
		mt9d131_write(0xf1, loop);  /* MCU_DATA_0  */  


		mt9d131_write(0xC6, 0xA1);
		mt9d131_write(0xf1, 0x03);  /* MCU_ADDRESS [SEQ_CMD] */
		mt9d131_write(0xC8, 0x00);
		mt9d131_write(0xf1, 0x05); 
	}
	else{
		printf("contrast set:invalid value %d/%d\r\n", val, max_value.ContrastMax);
	}
}
void MT9D131_set_exposure(unsigned char val){}
void MT9D131_set_sharpness(unsigned char val){}
void MT9D131_color_mode(unsigned char mode)
{
	switch(mode){
		case COLOR_MODE_BW:
			mt9d131_write(0xf0, 0x00);
			mt9d131_write(0xf1, 0x01);//ser page 1

			mt9d131_write(0x97, 0x00);
			mt9d131_write(0xf1, 0x08);
			break;
		case COLOR_MODE_NORMAL:
		default:
			mt9d131_write(0xf0, 0x00);
			mt9d131_write(0xf1, 0x01);//ser page 1

			mt9d131_write(0x97, 0x00);
			mt9d131_write(0xf1, 0x00);
			break;
	}
}
void MT9D131_reg_write(unsigned char addr,unsigned char val){}
unsigned char  MT9D131_reg_read(unsigned char addr){}

void SPEC_MT9D131_reg_write(unsigned char page, unsigned char addr,uint16_t val)
{
	uint8_t high_bit= 0;
	uint8_t low_bit = 0;
	uint8_t npage = page & 0x01;

	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, npage); //set page

	printf("mt9d131 reg write page%d %02x:%04x\r\n", npage, addr, val);
	low_bit = (unsigned char)(val & 0x00ff);
	high_bit = (unsigned char)(val>>8);
	mt9d131_write(addr, high_bit);
	mt9d131_write(0xf1, low_bit);

}
uint16_t SPEC_MT9D131_reg_read(unsigned char page, unsigned char addr)
{
	uint16_t high_bit= 0;
	uint8_t low_bit = 0;
	uint16_t ret_reg = 0;
	uint8_t npage = page & 0x01;
	
	mt9d131_write(0xf0, 0x00);
	mt9d131_write(0xf1, npage); //set page

	high_bit = mt9d131_read(addr);
	low_bit = mt9d131_read(0xf1); 

	ret_reg = (high_bit<<8)|low_bit;
	printf("mt9d131 reg read page%d %02x:%04x\r\n", npage, addr, ret_reg);
	return ret_reg;
}

ColorMaxValue MT9D131_get_color_max_value()
{
	ColorMaxValue value_info;
	value_info.HueMax = 63;
	value_info.SaturationMax = 255;
	value_info.ContrastMax= 4;
	value_info.BrightnessMax= 255;

	return value_info;
}

void MT9D131_set_shutter(unsigned char val)
{
	if(val == SYS_VIN_DIGITAL_SHUTTER_50HZ){//50Hz
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01);   //ser page 1

		mt9d131_write(0xC6, 0xa4);        
		mt9d131_write(0xf1, 0x04);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0xc0);

		mt9d131_write(0xC6, 0xa1);        
		mt9d131_write(0xf1, 0x03);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x06);
		printf("%s: 50Hz\r\n", __FUNCTION__);
	}else if(val == SYS_VIN_DIGITAL_SHUTTER_60HZ){//60Hz
		mt9d131_write(0xf0, 0x00);
		mt9d131_write(0xf1, 0x01);   //ser page 1

		mt9d131_write(0xC6, 0xa4);        
		mt9d131_write(0xf1, 0x04);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x80);

		mt9d131_write(0xC6, 0xa1);        
		mt9d131_write(0xf1, 0x03);
		mt9d131_write(0xC8, 0x00);       
		mt9d131_write(0xf1, 0x06);
		printf("%s: 60Hz\r\n", __FUNCTION__);
	}else{
		printf("%s:invalid value\r\n", __FUNCTION__);
	}
}

