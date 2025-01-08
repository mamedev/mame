// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    Siemens SDA5708  8 character 7x5 dot matrix LED display

**********************************************************************/

#ifndef MAME_VIDEO_SDA5708_H
#define MAME_VIDEO_SDA5708_H

#pragma once

/* Misc info
 * ---------
 * http://www.sbprojects.com/knowledge/footprints/sda5708/index.php
 * http://arduinotehniq.blogspot.se/2015/07/sda5708-display-8-character-7x5-dot.html
 *
 * Display front - LEDs
 * --------------------
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *           xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx  xxxxx
 *            Dig0   Dig1   Dig2   Dig3   Dig4   Dig5   Dig6   Dig7
 *
 * Display rear - Pinout
 * ---------------------
 *        +--------------------------------------------------------+
 *        | O                                                    O |
 *        |    +----------------------------------------------+    |
 *        |    |                  o o o o o o                 |    |
 *        |    |              Pin:6 5 4 3 2 1                 |    |
 *        |    |                                              |    |
 *        |    +----------------------------------------------+    |
 *        +--------------------------------------------------------+
 *             6:GND  5:_RESET  4:SDCLK  3:Data  2:_Load  1:Vcc
 *
 */


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> sda5708_device

class sda5708_device :  public device_t
{
public:
	// construction/destruction
	sda5708_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void load_w(int state);
	void data_w(int state);
	void sdclk_w(int state);
	void reset_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_display();

private:
	enum {
		SDA5708_REG_MASK       = 0xE0,

		SDA5708_CNTR_COMMAND   = 0xE0,
		SDA5708_CNTR_BRIGHT_MAS= 0x1F,
		SDA5708_CNTR_BRIGHT_100= 0x00,
		SDA5708_CNTR_BRIGHT_53 = 0x01,
		SDA5708_CNTR_BRIGHT_40 = 0x02,
		SDA5708_CNTR_BRIGHT_27 = 0x03,
		SDA5708_CNTR_BRIGHT_20 = 0x04,
		SDA5708_CNTR_BRIGHT_13 = 0x05,
		SDA5708_CNTR_BRIGHT_6_6= 0x06,
		SDA5708_CNTR_BRIGHT_0  = 0x07,
		SDA5708_CNTR_PKCUR_12_5= 0x08,

		SDA5708_CLEAR_COMMAND  = 0xC0,

		SDA5708_ADDR_COMMAND   = 0xA0,
		SDA5708_ADDR_LED_MASK  = 0x07,
		SDA5708_ADDR_LED0      = 0x00,
		SDA5708_ADDR_LED1      = 0x01,
		SDA5708_ADDR_LED2      = 0x02,
		SDA5708_ADDR_LED3      = 0x03,
		SDA5708_ADDR_LED4      = 0x04,
		SDA5708_ADDR_LED5      = 0x05,
		SDA5708_ADDR_LED6      = 0x06,
		SDA5708_ADDR_LED7      = 0x07,

		SDA5708_DATA_COMMAND   = 0x00
	};

	output_finder<8, 7, 5> m_dots;
	uint8_t m_serial;
	uint8_t m_load;
	uint8_t m_reset;
	uint8_t m_data;
	uint8_t m_sdclk;
	uint8_t m_dispmem[7 * 8];
	uint8_t m_cdp;
	uint8_t m_digit;
	uint8_t m_bright;
	uint8_t m_clear;
	uint8_t m_ip;
};


// device type definition
DECLARE_DEVICE_TYPE(SDA5708, sda5708_device)

#endif // MAME_VIDEO_SDA5708_H
