/** \file ds1820.c
 *  \brief Functions for DS18x20 communication
 *	This module is part of the EIB-LCD Controller Firmware
 *
 *	Implemented functions:
 *	- init and reset GPIO for DS18x20 communication
 *	- state machine communicating to DS18x20 temperature sensors
 *	- transformation of raw data to IEEE floating point value
 *	- transformation of floating point value into EIS5 value
 *
 *	Copyright (c) 2011-2013 Arno Stock <arno.stock@yahoo.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 */
#include "1wire_io.h"
#include "ds1820.h"
#include "hardware.h"
#include <util/crc16.h>

enum DS1820_STATES { DS1820_IDLE, DS1820_START_CONVERSION, DS1820_END_OF_CONVERSION, DS1820_READ0, DS1820_READ1, DS1820_READ2, DS1820_READ3,
							DS1820_READ4, DS1820_READ5, DS1820_READ6, DS1820_READ7, DS1820_READ8 };
// state machine states for all HW channels
enum DS1820_STATES ds1820_state[sizeof(port_bit)];
uint16_t	ds1820_timer[sizeof(port_bit)];
uint8_t ds1820_crc[sizeof(port_bit)];
double ds1820_temp[sizeof(port_bit)];
uint8_t ds1820_remain[sizeof(port_bit)];

//final static String repeatTimes[] = { "1s", "2s", "5s", "10s", "30s", "60s" };
const uint16_t DS1820_REPEAT_TIMES[] = { 33, 65, 152, 303, 909, 1818 };

void init_ds1820 (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;

	init_1wire (p->hw_channel);
	// set state machine to wait for next conversion task
	ds1820_state [p->hw_channel] = DS1820_IDLE;
	ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

}

void terminate_ds1820 (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;

	terminate_1wire (p->hw_channel);

}


uint8_t ds_ch;
uint16_t ds_byte;
int16_t w;
double r;


// this is the cyclic tick for the DS18S20 state machine
void ds18S20_statemachine (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;
	ds_ch = p->hw_channel;
	if (ds_ch > sizeof(port_bit))
		return;

	if (ds1820_timer[ds_ch] > 0)
		ds1820_timer[ds_ch]--;
	if (ds1820_timer[ds_ch] > 0)
		return;

	switch (ds1820_state[ds_ch]) {

		case DS1820_IDLE:
			// set reset request
			if (reset_1wire (ds_ch, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, ds_ch, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("DS No Slave %d\n"), ds_ch);
#endif
				break;
			}
			// issue conversion start
			ds1820_state [p->hw_channel] = DS1820_START_CONVERSION;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Slave %d ok\n"), ds_ch);
#endif
		break;
		case DS1820_START_CONVERSION:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, ds_ch);
			send_1wire_byte (DS1820_CMD_CONVERT, ds_ch);
			ds1820_state [p->hw_channel] = DS1820_END_OF_CONVERSION;
			ds1820_timer [p->hw_channel] = DS1820_CONVERSION_TICKS;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Conversion %d\n"), ds_ch);
#endif
		break;
		case DS1820_END_OF_CONVERSION:
			// set reset request
			if (reset_1wire (ds_ch, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];
				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, ds_ch, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("DS No Slave %d\n"), ds_ch);
#endif
				break;
			}
			// issue conversion start
			ds1820_state [p->hw_channel] = DS1820_READ0;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Slave %d ok\n"), ds_ch);
#endif
		break;
		case DS1820_READ0:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, ds_ch);
			send_1wire_byte (DS1820_CMD_READ, ds_ch);
			ds1820_crc[ds_ch] = 0;
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_temp[ds_ch] = ds_byte;
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ1;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ1:
			ds_byte = receive_1wire_byte(ds_ch);
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
			w= ds1820_temp[ds_ch];
			w |= (int16_t) (ds_byte << 8);
			ds1820_temp[ds_ch] = w;
			ds1820_temp[ds_ch] *= 0.5;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %4.4x = %2.2f C\n"), w, ds1820_temp[ds_ch]);
#endif
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ2;
		break;
		case DS1820_READ2:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ3;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ3:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ4;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ4:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ5;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ5:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ6;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ6:
			ds_byte = receive_1wire_byte(ds_ch);
			// remember for remainder calculation
			ds1820_remain[ds_ch] = ds_byte;
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ7;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ7:
			ds_byte = receive_1wire_byte(ds_ch);
			// this is count per C
			// add remainder

			r = ds_byte - ds1820_remain[ds_ch];
			ds1820_temp[ds_ch] += (-0.25 + r/ds_byte);

#ifdef HW_DEBUG
    printf_P(PSTR("DS With remainder %2.2f C\n"), ds1820_temp[ds_ch]);
#endif

			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ8;
#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ8:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);

#ifdef HW_DEBUG
    printf_P(PSTR("DS Read %2.2x from %d with CRC=%2.2x\n"), ds_byte, ds_ch, ds1820_crc[ds_ch]);
#endif

			ds1820_state [p->hw_channel] = DS1820_IDLE;
			ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

			// add temperature offset
			r = p->temp_offset;
			ds1820_temp[ds_ch] += r/10;
#ifdef HW_DEBUG
    printf_P(PSTR("DS With offset %2.2f C\n"), ds1820_temp[ds_ch]);
#endif

			// forward to HW monitor
			hwmon_show_ds1820_event (ds1820_temp[ds_ch], r, ds_ch, ds1820_crc[ds_ch], 1);

			// check CRC
			if (!ds1820_crc[ds_ch]) {

				// convert value and sent it to bus
				switch (p->eis_number_format) {
					case DS1820_FORMAT_EIS5:

						ds1820_temp[ds_ch] *= 100.0/8.0;
						w = (int16_t) ds1820_temp[ds_ch];
						w &= 0x07ff;
						w |= (3 << 11);
						if (ds1820_temp[ds_ch] < 0)
							w |= 0x8000;
						// exchange LB-HB
						ds_byte = (w >> 8) & 0xff;
						w = w << 8;
						w |= ds_byte;
						ds_byte = p->eib_object;
						eib_G_DATA_request(get_group_address (ds_byte), (uint8_t*)&w, 2);
					break;
				}

			}
			else {
				// next conversion after 1 sec.
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [0];
			}


		break;

		// should never happen
		default: ds1820_state[ds_ch] = DS1820_IDLE;
#ifdef HW_DEBUG
    printf_P(PSTR("DS ERROR: default state hit for %d\n"), ds_ch);
#endif
	}
}


// this is the cyclic tick for the DS18B20 state machine
void ds18B20_statemachine (char *cp) {

_O_DS1820_t*	p;

	p = (_O_DS1820_t*) cp;
	ds_ch = p->hw_channel;
	if (ds_ch > sizeof(port_bit))
		return;

	if (ds1820_timer[ds_ch] > 0)
		ds1820_timer[ds_ch]--;
	if (ds1820_timer[ds_ch] > 0)
		return;

	switch (ds1820_state[ds_ch]) {

		case DS1820_IDLE:
			// set reset request
			if (reset_1wire (ds_ch, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, ds_ch, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("DSB No Slave %d\n"), ds_ch);
#endif
				break;
			}
			// issue conversion start
			ds1820_state [p->hw_channel] = DS1820_START_CONVERSION;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Slave %d ok\n"), ds_ch);
#endif
		break;
		case DS1820_START_CONVERSION:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, ds_ch);
			send_1wire_byte (DS1820_CMD_CONVERT, ds_ch);
			ds1820_state [p->hw_channel] = DS1820_END_OF_CONVERSION;
			ds1820_timer [p->hw_channel] = DS1820_CONVERSION_TICKS;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Conversion %d\n"), ds_ch);
#endif
		break;
		case DS1820_END_OF_CONVERSION:
			// set reset request
			if (reset_1wire (ds_ch, 0)) {
				// the sensor is not ready
				ds1820_state [p->hw_channel] = DS1820_IDLE;
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];
				// forward to HW monitor
				hwmon_show_ds1820_event (0, 0, ds_ch, 0, 0);

#ifdef HW_DEBUG
    printf_P(PSTR("DSB No Slave %d\n"), ds_ch);
#endif
				break;
			}
			// issue data read start
			ds1820_state [p->hw_channel] = DS1820_READ0;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Slave %d ok\n"), ds_ch);
#endif
		break;
		case DS1820_READ0:
			send_1wire_byte (DS1820_CMD_SKIP_ROM, ds_ch);
			send_1wire_byte (DS1820_CMD_READ, ds_ch);
			ds1820_crc[ds_ch] = 0;
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_temp[ds_ch] = ds_byte;
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ1;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ1:
			ds_byte = receive_1wire_byte(ds_ch);
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
			w= ds1820_temp[ds_ch];
			w |= ds_byte << 8;
			ds1820_temp[ds_ch] = w * 0.0625; // 12 bit resolution
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %4.4x = %2.2f C\n"), w, ds1820_temp[ds_ch]);
#endif
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ2;
		break;
		case DS1820_READ2:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ3;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ3:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ4;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ4:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ5;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ5:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ6;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ6:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ7;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ7:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);
			ds1820_state [p->hw_channel] = DS1820_READ8;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d\n"), ds_byte, ds_ch);
#endif
		break;
		case DS1820_READ8:
			ds_byte = receive_1wire_byte(ds_ch);
			ds1820_crc[ds_ch] = _crc_ibutton_update (ds1820_crc[ds_ch], ds_byte);

#ifdef HW_DEBUG
    printf_P(PSTR("DSB Read %2.2x from %d with CRC=%2.2x\n"), ds_byte, ds_ch, ds1820_crc[ds_ch]);
#endif

			ds1820_state [p->hw_channel] = DS1820_IDLE;
			ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [p->repeat_frequency];

			// add temperature offset
			r = p->temp_offset;
			ds1820_temp[ds_ch] += r/10;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB With offset %2.2f C\n"), ds1820_temp[ds_ch]);
#endif

			// forward to HW monitor
			hwmon_show_ds1820_event (ds1820_temp[ds_ch], r, ds_ch, ds1820_crc[ds_ch], 1);

			// check CRC
			if (!ds1820_crc[ds_ch]) {

				// convert value and sent it to bus
				switch (p->eis_number_format) {
					case DS1820_FORMAT_EIS5:

						// TEST, TODO
                        //ds1820_temp[ds_ch] = 0xFFFF * 0.0625 + 0.0625; // 12 bit resolution

                        printf_P(PSTR("DSB DEBUG 1     %10.5f\n"), ds1820_temp[ds_ch]);

                        ds1820_temp[ds_ch] *= 100.0/8.0;
                        printf_P(PSTR("DSB DEBUG 2     %10.5f\n"), ds1820_temp[ds_ch]);
						w = (int16_t) ds1820_temp[ds_ch];
                        printf_P(PSTR("DSB DEBUG 3     %d, = %4.4x\n"), w,w);
                        //w &= 0x07ff;    // mantisse


                        if (ds1820_temp[ds_ch] < 0) // TODO -0.06 auf dem LCD -> -163.84 -> 9800
                        {
                            w--;
                            printf_P(PSTR("DSB DEBUG 4A    %d, = %4.4x\n"), w,w);

                            w &= 0x07ff;    // mantisse
                            printf_P(PSTR("DSB DEBUG 4B    %d, = %4.4x\n"), w,w);
                            w |= 0x8000;    // mantisse pos/neg
                        }
                        else
                            w &= 0x07ff;    // mantisse

                        printf_P(PSTR("DSB DEBUG 5     %d, = %4.4x\n"), w,w);
						w |= (3 << 11); // Exponent 3 fix
                        printf_P(PSTR("DSB DEBUG 6     %d, = %4.4x\n"), w,w);

						// exchange LB-HB
						ds_byte = (w >> 8) & 0xff;
						w = w << 8;
						w |= ds_byte;
						ds_byte = p->eib_object;
						eib_G_DATA_request(get_group_address (ds_byte), (uint8_t*)&w, 2);
					break;
				}

			}
			else {
				// next conversion after 1 sec.
				ds1820_timer [p->hw_channel] = DS1820_REPEAT_TIMES [0];
			}


		break;

		// should never happen
		default: ds1820_state[ds_ch] = DS1820_IDLE;
#ifdef HW_DEBUG
    printf_P(PSTR("DSB ERROR: default state hit for %d\n"), ds_ch);
#endif
	}
}
