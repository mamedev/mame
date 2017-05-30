// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    Siemens SDA5708  8 character 7x5 dot matrix LED display

**********************************************************************/

#pragma once

#ifndef __SDA5708__
#define __SDA5708__

#include "emu.h"



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> sda5708_device

class sda5708_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	sda5708_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ8_MEMBER( ir_r );
	DECLARE_WRITE8_MEMBER( ir_w );

	DECLARE_READ8_MEMBER( dr_r );
	DECLARE_WRITE8_MEMBER( dr_w );

	UINT32 screen_update(screen_device &device, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

private:
	void update_cursor();
	void draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, UINT16 ma, UINT8 ra = 0);
	void update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_text(bitmap_ind16 &bitmap, const rectangle &cliprect);

	const address_space_config m_space_config;

	int m_frame;                    // frame counter
};


// device type definition
extern const device_type SDA5708;



#endif
