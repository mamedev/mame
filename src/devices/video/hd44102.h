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
	hd44102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	static void static_set_offsets(device_t &device, int sx, int sy);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void cs2_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
extern const device_type HD44102;



#endif
