// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    divideo.h

    Device video interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIVIDEO_H__
#define __DIVIDEO_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VIDEO_SET_SCREEN(_tag) \
	device_video_interface::static_set_screen(*device, _tag);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_video_interface

class device_video_interface : public device_interface
{
	static const char s_unconfigured_screen_tag[];

public:
	// construction/destruction
	device_video_interface(const machine_config &mconfig, device_t &device, bool screen_required = true);
	virtual ~device_video_interface();

	// static configuration
	static void static_set_screen(device_t &device, std::string tag);

	// getters
	screen_device &screen() const { return *m_screen; }

protected:
	// optional operation overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

	// configuration state
	bool            m_screen_required;          // is a screen required?
	std::string     m_screen_tag;               // configured tag for the target screen

	// internal state
	screen_device * m_screen;                   // pointer to the screen device
};

// iterator
typedef device_interface_iterator<device_video_interface> video_interface_iterator;


#endif  /* __DIVIDEO_H__ */
