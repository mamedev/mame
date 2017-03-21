// license:BSD-3-Clause
// copyright-holders:AJR
/******************************************************************************

    Video System C7-01 GGA

******************************************************************************/

#pragma once

#ifndef MAME_VIDEO_VSYSTEM_GGA_H
#define MAME_VIDEO_VSYSTEM_GGA_H

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VSYSTEM_GGA_REGISTER_WRITE_CB(_devcb) \
	devcb = &vsystem_gga_device::static_set_write_cb(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vsystem_gga_device

class vsystem_gga_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	vsystem_gga_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// static configuration
	template<class Obj> static devcb_base &static_set_write_cb(device_t &device, Obj &&object) { return downcast<vsystem_gga_device &>(device).m_write_cb.set_callback(std::forward<Obj>(object)); }

	// memory handlers
	DECLARE_WRITE8_MEMBER(write);

	// temporary accessor
	u8 reg(u8 offset) const { return m_regs[offset]; }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	u8 m_address_latch;
	u8 m_regs[16];
	devcb_write8 m_write_cb;
};

// device type definition
extern const device_type VSYSTEM_GGA;

#endif
