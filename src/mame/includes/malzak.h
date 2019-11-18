// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*************************************************************************

    Malzak

*************************************************************************/
#ifndef MAME_INCLUDES_MALZAK_H
#define MAME_INCLUDES_MALZAK_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "video/saa5050.h"
#include "emupal.h"
#include "screen.h"

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
	virtual void machine_start() override;
	virtual void machine_reset() override;

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
	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int m_playfield_code[256];
	int m_malzak_x;
	int m_malzak_y;
	int m_collision_counter;

	DECLARE_READ8_MEMBER(fake_VRLE_r);
	DECLARE_READ8_MEMBER(s2636_portA_r);
	DECLARE_READ8_MEMBER(s2650_data_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(port60_w);
	DECLARE_WRITE8_MEMBER(portc0_w);
	DECLARE_READ8_MEMBER(collision_r);
	DECLARE_WRITE8_MEMBER(malzak_playfield_w);
	DECLARE_READ8_MEMBER(videoram_r);

	void malzak_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void malzak2_map(address_map &map);
	void malzak_data_map(address_map &map);
	void malzak_io_map(address_map &map);
	void malzak_map(address_map &map);
};

#endif // MAME_INCLUDES_MALZAK_H
