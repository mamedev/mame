// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver emulation

**********************************************************************/

#pragma once

#ifndef __HD44102__
#define __HD44102__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_HD44102_ADD(_tag, _screen_tag, _sx, _sy) \
	MCFG_DEVICE_ADD(_tag, HD44102, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	hd44102_device::static_set_offsets(*device, _sx, _sy);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> hd44102_device

class hd44102_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	hd44102_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_offsets(device_t &device, int sx, int sy);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( cs2_w );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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

	UINT8 m_ram[4][50];             // display memory

	UINT8 m_status;                 // status register
	UINT8 m_output;                 // output register

	int m_cs2;                      // chip select
	int m_page;                     // display start page
	int m_x;                        // X address
	int m_y;                        // Y address

	int m_sx;
	int m_sy;
};


// device type definition
extern const device_type HD44102;



#endif
