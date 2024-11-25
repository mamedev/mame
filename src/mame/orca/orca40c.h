// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

/***************************************************************************

    orca40c.h

***************************************************************************/

#ifndef MAME_ORCA_ORCA40C_H
#define MAME_ORCA_ORCA40C_H

#pragma once

#include "emupal.h"
#include "tilemap.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> orca_ovg_40c_device

class orca_ovg_40c_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	// construction/destruction
	orca_ovg_40c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_percuss_hardware(bool percuss_hardware) { m_percuss_hardware = percuss_hardware; }

	// memory handlers
	void videoram_w(offs_t offset, uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	void attributes_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// shared memory finders
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram_2;
	required_shared_ptr<uint8_t> m_attributeram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bulletsram;

	// configuration
	bool m_percuss_hardware;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// internal state
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	bool m_flip_screen;

	// helpers
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette_init(palette_device &palette);

	// drawing control
	void draw_bullets(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

// device type definition
DECLARE_DEVICE_TYPE(ORCA_OVG_40C, orca_ovg_40c_device)

#endif  // MAME_ORCA_ORCA40C_H
