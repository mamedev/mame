// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna
#ifndef MAME_INCLUDES_WRALLY_H
#define MAME_INCLUDES_WRALLY_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"
#include "video/gaelco_wrally_sprites.h"

class wrally_state : public driver_device
{
public:
	wrally_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprites(*this, "sprites"),
		m_okibank(*this, "okibank"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_tilemap{ nullptr, nullptr }
	{
	}

	void wrally(machine_config &config);

private:
	DECLARE_READ8_MEMBER(shareram_r);
	DECLARE_WRITE8_MEMBER(shareram_w);
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(okim6295_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void video_start() override;
	void mcu_hostmem_map(address_map &map);
	void oki_map(address_map &map);
	void wrally_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<gaelco_wrally_sprites_device> m_sprites;
	required_memory_bank m_okibank;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;

	tilemap_t *m_tilemap[2];
};

#endif // MAME_INCLUDES_WRALLY_H
