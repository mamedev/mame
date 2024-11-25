// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow LCD Display

**********************************************************************/

#ifndef MAME_BUS_BBC_USERPORT_LCD_H
#define MAME_BUS_BBC_USERPORT_LCD_H

#pragma once

#include "userport.h"
#include "video/hd44780.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_lcd_device

class bbc_lcd_device :
	public device_t,
	public device_bbc_userport_interface
{
public:
	// construction/destruction
	bbc_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;

private:
	required_device<hd44780_device> m_lcdc;

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void lcd_palette(palette_device &palette) const;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_LCD, bbc_lcd_device)


#endif // MAME_BUS_BBC_USERPORT_LCD_H
