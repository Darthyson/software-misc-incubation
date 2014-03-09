/**
 * \file tft_ssd1963_43_0.h
 *
 * \brief This module contains constant definitions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#ifndef TFT_SSD1963_43_0_H_
#define TFT_SSD1963_43_0_H_

#include "tft_io.h"

// Touch offset values for new 4.3" LCD (Type 2)
// Measured x --> ly and Y_OFFSET
// Measured y --> lx and X_OFFSET
#define SSD1963_43_Y_OFFSET			390		//340
#define SSD1963_43_Y_OFFSET_FACT	11.8	//12.3
#define SSD1963_43_X_OFFSET			250		//170
#define SSD1963_43_X_OFFSET_FACT	7.6		//7.7

#define		T2_HDP		479	//-->480
#define		T2_HT		531
#define		T2_HPS		43
#define 	T2_LPS		8
#define 	T2_HPW		10

#define		T2_VDP		271	//-->272
#define 	T2_VT		288
#define		T2_VPS		12
#define		T2_FPS		4
#define		T2_VPW		10

#include "ssd1963_cmd.h"

//SSL[15:8] : Supplier ID of Solomon Systech Limited high byte, always 01h (POR = 00000001) 
#define SSD1963_SSL_H	0x01
//SSL[7:0] : Supplier ID of Solomon Systech Limited low byte, always 57h (POR = 010101110) 
#define SSD1963_SSL_L	0x57
//PROD[7:0] : Product ID, always 61h (POR = 01100001) 
#define	SSD1963_PROD	0x61
//REV[2:0] : Revision code, always 01h (POR = 001) 
#define	SSD1963_REV		0x01
//Exit code, always FFh (POR = 11111111) 			
#define	SSD1963_EXIT	0xff

void ssd1963_43_0_init(void);

#endif //TFT_SSD1963_43_0_H_
