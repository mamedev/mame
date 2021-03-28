// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    The Sega 3-D Glasses / SegaScope 3-D Glasses emulation

**********************************************************************/

#ifndef MAME_BUS_SMS_3D_GLASSES_H
#define MAME_BUS_SMS_3D_GLASSES_H

#pragma once


#include "crsshair.h"
#include "video/315_5124.h"
#include "s3dport.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sms_3d_glasses_device

class sms_3d_glasses_device : public device_t,
							public device_sms_3d_port_interface
{
public:
	// construction/destruction
	sms_3d_glasses_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void update_displayed_range() override;

	// optional information overrides
	ioport_constructor device_input_ports() const override;

private:
	template <typename X> static void screen_sms_ntsc_raw_params(screen_device &screen, X &&pixelclock);
	template <typename X> static void screen_sms_3d_glasses_raw_params(screen_device &screen, sms_3d_port_device *port, X &&pixelclock);

	void blit_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_left_lcd;
	required_device<screen_device> m_right_lcd;

	bitmap_rgb32 m_tmpbitmap;

	// for 3D glass binocular hack
	required_ioport m_port_binocular;
	bitmap_rgb32 m_prevleft_bitmap;
	bitmap_rgb32 m_prevright_bitmap;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_3D_GLASSES, sms_3d_glasses_device)


#endif // MAME_BUS_SMS_3D_GLASSES_H
