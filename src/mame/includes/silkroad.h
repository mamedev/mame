// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
#ifndef MAME_INCLUDES_SILKROAD_H
#define MAME_INCLUDES_SILKROAD_H

#pragma once

#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class silkroad_state : public driver_device
{
public:
	silkroad_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram%u", 1U),
		m_sprram(*this, "sprram"),
		m_regs(*this, "regs"),
		m_okibank(*this, "okibank")
	{ }

	void silkroad(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint32_t, 3> m_vram;
	required_shared_ptr<uint32_t> m_sprram;
	required_shared_ptr<uint32_t> m_regs;

	required_memory_bank m_okibank;

	tilemap_t *m_tilemap[3];

	DECLARE_WRITE8_MEMBER(coin_w);
	template<int Layer> DECLARE_WRITE32_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(okibank_w);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpu_map(address_map &map);
	void oki_map(address_map &map);
};

#endif // MAME_INCLUDES_SILKROAD_H
