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

#ifndef MAME_EMU_DIVIDEO_H
#define MAME_EMU_DIVIDEO_H


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

	// configuration
	void set_screen(const char *tag);
	void set_screen(device_t &base, const char *tag)
	{
		m_screen_base = &base;
		m_screen_tag = tag;
	}
	template <class ObjectClass, bool Required>
	void set_screen(device_finder<ObjectClass, Required> &finder)
	{
		m_screen_base = &finder.finder_target().first;
		m_screen_tag = finder.finder_target().second;
	}

	// getters
	screen_device &screen() const { return *m_screen; }
	bool has_screen() const { return m_screen != nullptr; }

protected:
	// optional operation overrides
	virtual void interface_config_complete() override;
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

private:
	// configuration state
	bool            m_screen_required;          // is a screen required?
	device_t *      m_screen_base;              // base device for resolving target screen
	const char *    m_screen_tag;               // configured tag for the target screen

	// internal state
	screen_device * m_screen;                   // pointer to the screen device
};

// iterator
typedef device_interface_iterator<device_video_interface> video_interface_iterator;


#endif  /* MAME_EMU_DIVIDEO_H */
