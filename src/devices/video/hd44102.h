// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver

**********************************************************************/

#ifndef MAME_VIDEO_HD44102_H
#define MAME_VIDEO_HD44102_H

#pragma once


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> hd44102_device

class hd44102_device : public device_t
{
public:
	// construction/destruction
	hd44102_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// inline configuration helpers
	void set_screen_offsets(int sx, int sy) { m_sx = sx; m_sy = sy; } // when using screen_update

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	const u8 *render();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect); // optional

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 status_r();
	void control_w(u8 data);

	void count_up_or_down();
	u8 data_r();
	void data_w(u8 data);

	u8 m_ram[4][50];            // display memory
	u8 m_render_buf[32 * 50];   // intermediate pixel buffer

	u8 m_status;                // status register
	u8 m_output;                // output register

	u8 m_page;                  // display start page
	int m_x;                    // X address
	int m_y;                    // Y address

	int m_sx;
	int m_sy;
};


// device type definition
DECLARE_DEVICE_TYPE(HD44102, hd44102_device)

#endif // MAME_VIDEO_HD44102_H
