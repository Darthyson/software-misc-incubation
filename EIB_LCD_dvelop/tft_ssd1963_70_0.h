/**
 * \file tft_ssd1963_70_0.h
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
#ifndef TFT_SSD1963_70_0_H_
#define TFT_SSD1963_70_0_H_

#include "tft_io.h"

// Touch offset values for 7.0" LCD (Type 3)
// Measured x --> ly and Y_OFFSET
// Measured y --> lx and X_OFFSET
#define SSD1963_70_Y_OFFSET			360	//3800, 360
#define SSD1963_70_Y_OFFSET_FACT	7.2
#define SSD1963_70_X_OFFSET			130 //3900
#define SSD1963_70_X_OFFSET_FACT	4.7

#define		T3_HDP		799	//-->800
#define		T3_HT		928
#define		T3_HPS		46
#define 	T3_LPS		15
#define 	T3_HPW		48

#define		T3_VDP		479	//-->480
#define 	T3_VT		525
#define		T3_VPS		16
#define		T3_FPS		8
#define		T3_VPW		16

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

void ssd1963_70_0_init(void);

#endif //TFT_SSD1963_70_0_H_
