// license:BSD-3-Clause
// copyright-holders: Allard van der Bas

/***************************************************************************

    vastar_viddev.h

***************************************************************************/

#ifndef MAME_ORCA_VASTAR_VIDDEV_H
#define MAME_ORCA_VASTAR_VIDDEV_H

#pragma once

#include "emupal.h"
#include "tilemap.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vastar_video_device

class vastar_video_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	// construction/destruction
	vastar_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_bg_bases(uint16_t code, uint16_t attr, uint16_t col) { m_bg_codebase = code; m_bg_attrbase = attr;  m_bg_colbase = col;  }
	void set_fg_bases(uint16_t code, uint16_t attr, uint16_t col) { m_fg_codebase = code; m_fg_attrbase = attr;  m_fg_colbase = col;  }
	void set_other_bases(uint16_t spy, uint16_t atr, uint16_t spx, uint16_t bgs0, uint16_t bgs1) { m_spr_y_col = spy; m_spr_attr = atr; m_spr_code_x = spx; m_bg_scroll0 = bgs0; m_bg_scroll1 = bgs1; }
	void set_alt_sprite_flips(bool alt_flip) { m_alt_spriteflip = alt_flip; }

	template <typename... T> void set_bg0ram_tag(T &&... args) { m_bgvideoram[0].set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_bg1ram_tag(T &&... args) { m_bgvideoram[1].set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_fgram_tag(T &&... args) { m_fgvideoram.set_tag(std::forward<T>(args)...); }

	// memory handlers
	void fgvideoram_w(offs_t offset, uint8_t data)
	{
		m_fgvideoram[offset] = data;
		m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
	}

	template <uint8_t Which>
	void bgvideoram_w(offs_t offset, uint8_t data)
	{
		m_bgvideoram[Which][offset] = data;
		m_bg_tilemap[Which]->mark_tile_dirty(offset & 0x3ff);
	}

	void flipscreen_w(int state);
	void priority_w(uint8_t data) { m_fg_vregs = data; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// shared memory finders
	required_shared_ptr_array<uint8_t, 2> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// internal state
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};

	uint8_t m_fg_vregs;
	uint8_t m_flip_screen;

	// config
	uint16_t m_bg_codebase = 0;
	uint16_t m_bg_attrbase = 0;
	uint16_t m_bg_colbase = 0;
	uint16_t m_fg_codebase = 0;
	uint16_t m_fg_attrbase = 0;
	uint16_t m_fg_colbase = 0;

	uint16_t m_spr_y_col = 0;
	uint16_t m_spr_attr = 0;
	uint16_t m_spr_code_x = 0;
	uint16_t m_bg_scroll0 = 0;
	uint16_t m_bg_scroll1 = 0;

	bool m_alt_spriteflip = false;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t rambase, uint16_t tilebase );
};

// device type definition
DECLARE_DEVICE_TYPE(VASTAR_VIDEO_DEVICE, vastar_video_device)

#endif  // MAME_ORCA_VASTAR_VIDDEV_H
