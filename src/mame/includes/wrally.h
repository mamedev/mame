// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna
#ifndef MAME_INCLUDES_WRALLY_H
#define MAME_INCLUDES_WRALLY_H

#pragma once

#include "machine/74259.h"
#include "machine/gaelcrpt.h"
#include "video/gaelco_wrally_sprites.h"
#include "emupal.h"
#include "tilemap.h"

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
		m_vramcrypt(*this, "vramcrypt"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_analog(*this, "ANALOG%u", 0U),
		m_tilemap{ nullptr, nullptr }
	{
	}

	void wrally(machine_config &config);

	template <int N> DECLARE_READ_LINE_MEMBER(analog_bit_r);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	uint8_t shareram_r(offs_t offset);
	void shareram_w(offs_t offset, uint8_t data);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	void okim6295_bankswitch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);

	DECLARE_WRITE_LINE_MEMBER(adc_clk);
	DECLARE_WRITE_LINE_MEMBER(adc_en);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_hostmem_map(address_map &map);
	void oki_map(address_map &map);
	void wrally_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<gaelco_wrally_sprites_device> m_sprites;
	required_memory_bank m_okibank;
	required_device<gaelco_vram_encryption_device> m_vramcrypt;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;

	required_ioport_array<2> m_analog;

	tilemap_t *m_tilemap[2];
	uint8_t m_analog_ports[2];
};

#endif // MAME_INCLUDES_WRALLY_H
