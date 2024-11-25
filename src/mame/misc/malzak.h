// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*************************************************************************

    Malzak

*************************************************************************/
#ifndef MAME_MISC_MALZAK_H
#define MAME_MISC_MALZAK_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "video/saa5050.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class malzak_state : public driver_device
{
public:
	malzak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s2636(*this, "s2636%u", 0U)
		, m_trom(*this, "saa5050")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_mainbank(*this, "mainbank")
	{ }

	void malzak(machine_config &config);
	void malzak2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* devices */
	required_device<s2650_device> m_maincpu;
	required_device_array<s2636_device, 2> m_s2636;
	required_device<saa5050_device> m_trom;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_bank m_mainbank;

	uint8_t fake_VRLE_r();
	uint8_t s2636_portA_r();
	uint8_t s2650_data_r();
	void port40_w(uint8_t data);
	void port60_w(uint8_t data);
	void portc0_w(uint8_t data);
	uint8_t collision_r();
	void playfield_w(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);

	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void video_start() override ATTR_COLD;

	void malzak2_map(address_map &map) ATTR_COLD;
	void malzak_data_map(address_map &map) ATTR_COLD;
	void malzak_io_map(address_map &map) ATTR_COLD;
	void malzak_map(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_tile_info);
	std::unique_ptr<bitmap_rgb32> m_trom_bitmap;
	std::unique_ptr<bitmap_rgb32> m_playfield_bitmap;
	tilemap_t *m_playfield_tilemap = nullptr;
	int m_playfield_code[256]{};
	int m_scrollx = 0;
	int m_scrolly = 0;
	int m_collision_counter = 0;
	u8  m_playfield_bank = 0;
};

#endif // MAME_MISC_MALZAK_H
