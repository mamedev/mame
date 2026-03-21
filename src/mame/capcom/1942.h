// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Couriersud
/***************************************************************************

    1942

***************************************************************************/
#ifndef MAME_CAPCOM_1942_H
#define MAME_CAPCOM_1942_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"

#include "emupal.h"
#include "tilemap.h"
#include "screen.h"

class _1942_state : public driver_device
{
public:
	_1942_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_soundlatch(*this, "soundlatch")
		, m_spriteram(*this, "spriteram")
		, m_fg_videoram(*this, "fg_videoram")
		, m_bg_videoram(*this, "bg_videoram")
		, m_irqprom(*this, "irqprom")
	{ }

	void driver_init();

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void _1942(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void video_start() override ATTR_COLD;

	void _1942_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	void bankswitch_w(uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void palette_bank_w(uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	void _1942_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	optional_region_ptr<uint8_t> m_irqprom;

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	int m_palette_bank = 0;
	uint8_t m_scroll[2]{};
	void create_palette(palette_device &palette) const;
	uint8_t m_sprite_bufs[2][512]{};
};

class _1942p_state : public _1942_state
{
public:
	_1942p_state(const machine_config &mconfig, device_type type, const char *tag)
		: _1942_state(mconfig, type, tag)
		, m_protopal(*this, "protopal")
	{ }

	void _1942p(machine_config &config);

protected:
	void video_start() override ATTR_COLD;

	void _1942p_map(address_map &map) ATTR_COLD;
	void _1942p_sound_io(address_map &map) ATTR_COLD;
	void _1942p_sound_map(address_map &map) ATTR_COLD;

	void palette_w(offs_t offset, uint8_t data);

	void _1942p_palette(palette_device &palette) const;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	required_shared_ptr<uint8_t> m_protopal;
};

#endif // MAME_CAPCOM_1942_H
