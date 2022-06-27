// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Peter Ferrie
#ifndef MAME_INCLUDES_THOOP2_H
#define MAME_INCLUDES_THOOP2_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"
#include "tilemap.h"

class thoop2_state : public driver_device
{
public:
	thoop2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pant{ nullptr, nullptr },
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_okibank(*this, "okibank"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram")
	{ }

	void thoop2(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	void oki_bankswitch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);

	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void shareram_w(offs_t offset, uint8_t data);
	uint8_t shareram_r(offs_t offset);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_hostmem_map(address_map &map);
	void oki_map(address_map &map);
	void thoop2_map(address_map &map);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	tilemap_t *m_pant[2]{};

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_okibank;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;
};

#endif // MAME_INCLUDES_THOOP2_H
