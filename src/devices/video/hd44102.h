// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver emulation

**********************************************************************/

#ifndef MAME_VIDEO_HD44102_H
#define MAME_VIDEO_HD44102_H

#pragma once


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> hd44102_device

class hd44102_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	hd44102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag, int sx, int sy)
		:hd44102_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
		set_offsets(sx, sy);
	}

	hd44102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void set_offsets(int sx, int sy) { m_sx = sx; m_sy = sy; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( cs2_w );

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( control_w );

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	inline void count_up_or_down();

	uint8_t m_ram[4][50];             // display memory

	uint8_t m_status;                 // status register
	uint8_t m_output;                 // output register

	int m_cs2;                      // chip select
	int m_page;                     // display start page
	int m_x;                        // X address
	int m_y;                        // Y address

	int m_sx;
	int m_sy;
};


// device type definition
DECLARE_DEVICE_TYPE(HD44102, hd44102_device)

#endif // MAME_VIDEO_HD44102_H
