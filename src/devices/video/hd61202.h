// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

        Hitachi HD61202 LCD Driver

**********************************************************************/


#ifndef MAME_VIDEO_HD61202_H
#define MAME_VIDEO_HD61202_H

#pragma once

#define HD61202_UPDATE_CB(name) uint32_t name(bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, int start_line, uint8_t *ddr)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hd61202_device

class hd61202_device :  public device_t
{
public:
	typedef device_delegate<uint32_t (bitmap_ind16 &bitmap, const rectangle &cliprect, bool lcd_on, int start_line, uint8_t *ddr)> screen_update_delegate;

	// construction/destruction
	hd61202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	template <typename... T> void set_screen_update_cb(T &&... args) { m_screen_update_cb.set(std::forward<T>(args)...); }

	uint8_t status_r();
	uint8_t data_r();
	void control_w(uint8_t data);
	void data_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	screen_update_delegate  m_screen_update_cb;

	uint8_t m_bf;                   // busy flag
	uint8_t m_lcd_on;
	uint8_t m_out_data;
	uint8_t m_page;
	uint8_t m_addr;
	uint8_t m_start_line;
	uint8_t m_ddr[0x200];           // 4096 bit Display data RAM
};


DECLARE_DEVICE_TYPE(HD61202, hd61202_device)

#endif // MAME_VIDEO_HD61202_H
