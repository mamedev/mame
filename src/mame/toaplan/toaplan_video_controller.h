// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

Toaplan Video controller

***************************************************************************/
#ifndef MAME_TOAPLAN_TOAPLAN_VIDEO_CONTROLLER_H
#define MAME_TOAPLAN_TOAPLAN_VIDEO_CONTROLLER_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class toaplan_video_controller_device : public device_t,
							public device_video_interface
{
public:
	toaplan_video_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	template <typename T> void set_palette_tag(int layer, T &&tag) { m_palette[layer].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_paletteram_tag(int layer, T &&tag) { m_paletteram[layer].set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank_cb.bind(); }
	void set_byte_per_color(int layer, u8 byte)
	{
		m_byte_per_color[layer] = byte;
	}

	// host interfaces
	void intenable_w(offs_t offset, u8 data);
	void vtiming_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	template <unsigned Layer> u16 palette_r(offs_t offset);
	template <unsigned Layer> void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// getters
	bool intenable() { return m_intenable; }

	void host_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// devices
	required_device_array<palette_device, 2> m_palette;

	// memory pointers
	required_shared_ptr_array<u16, 2> m_paletteram;
	required_shared_ptr<u16> m_vtiming;

	// configurations
	devcb_read16 m_vblank_cb;
	u8 m_byte_per_color[2];

	// internal states
	u16 m_intenable;
};

DECLARE_DEVICE_TYPE(TOAPLAN_VIDEO_CONTROLLER, toaplan_video_controller_device)

#endif // MAME_TOAPLAN_TOAPLAN_VIDEO_CONTROLLER_H
