/**
 * \file tft_ssd1289_32_0.c
 *
 * \brief This module contains basic IO functions for the TFT LCD access
 * This module is part of the EIB-LCD Controller Firmware
 *
 * Controller:	SSD1289
 * Size:		3.2"
 * Resolution:	320 x 240
 * Order codes:	200475566068
 *
 *	Copyright (c) 2011-2014 Arno Stock <arno.stock@yahoo.de>
 *	Copyright (c) 2013-2014 Stefan Haller <stefanhaller.sverige@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "tft_ssd1289_32_0.h"


void ssd1289_32_0_address_set(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {

	main_W_com_data(0x44, (y2 << 8) + y1);
	main_W_com_data(0x45, x1);
	main_W_com_data(0x46, x2);
	main_W_com_data(0x4e, y1);
	main_W_com_data(0x4f, x1);

	tft_set_pointer(0x22);
}


void ssd1289_32_0_convert_touch_coordinates (void) {

	ly = (TP_Y - SSD1289_Y_OFFSET) / SSD1289_Y_OFFSET_FACT;

	if (invert_touch_x) {
		if (TP_X >= 380) {
			lx = (3910 - TP_X) / SSD1289_X_OFFSET_FACT;
			if (lx < 0)
				lx = 0;
		} else
			lx = (TP_X - 391) / SSD1289_X_OFFSET_FACT;

	} else
		lx = (TP_X - 350) / SSD1289_X_OFFSET_FACT;

}

void ssd1289_32_0_init() {

	// set global information
	drv_convert_touch_coordinates = ssd1289_32_0_convert_touch_coordinates;
	drv_address_set = ssd1289_32_0_address_set;
	// Return used resolution
	screen_max_x = 319;	// X
	screen_max_y = 239;	// Y

	main_W_com_data(0x0000, 0x0001);
	NutDelay(2);
	main_W_com_data(0x0003, 0xA8A4);
	NutDelay(2);
	main_W_com_data(0x000C, 0x0000);
	NutDelay(2);
	main_W_com_data(0x000D, 0x080C);
	NutDelay(2);
	main_W_com_data(0x000E, 0x2B00);
	NutDelay(2);
	main_W_com_data(0x001E, 0x00B0);
	NutDelay(2);
	main_W_com_data(0x0001, 0x293F);
	NutDelay(2); //320*240  0x6B3F
	main_W_com_data(0x0002, 0x0600);
	NutDelay(2);
	main_W_com_data(0x0010, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0011, 0x6078);
	NutDelay(2); //0x4030
	main_W_com_data(0x0005, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0006, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0016, 0xEF1C);
	NutDelay(2);
	main_W_com_data(0x0017, 0x0003);
	NutDelay(2);
	main_W_com_data(0x0007, 0x0233);
	NutDelay(2);
	main_W_com_data(0x000B, 0x0000);
	NutDelay(2);
	main_W_com_data(0x000F, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0041, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0042, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0048, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0049, 0x013F);
	NutDelay(2);
	main_W_com_data(0x004A, 0x0000);
	NutDelay(2);
	main_W_com_data(0x004B, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0044, 0xEF00);
	NutDelay(2);
	main_W_com_data(0x0045, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0046, 0x013F);
	NutDelay(2);
	main_W_com_data(0x0030, 0x0707);
	NutDelay(2);
	main_W_com_data(0x0031, 0x0204);
	NutDelay(2);
	main_W_com_data(0x0032, 0x0204);
	NutDelay(2);
	main_W_com_data(0x0033, 0x0502);
	NutDelay(2);
	main_W_com_data(0x0034, 0x0507);
	NutDelay(2);
	main_W_com_data(0x0035, 0x0204);
	NutDelay(2);
	main_W_com_data(0x0036, 0x0204);
	NutDelay(2);
	main_W_com_data(0x0037, 0x0502);
	NutDelay(2);
	main_W_com_data(0x003A, 0x0302);
	NutDelay(2);
	main_W_com_data(0x003B, 0x0302);
	NutDelay(2);
	main_W_com_data(0x0023, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0024, 0x0000);
	NutDelay(2);
	main_W_com_data(0x0025, 0x8000);
	NutDelay(2);
	main_W_com_data(0x004f, 0);
	main_W_com_data(0x004e, 0);
}
