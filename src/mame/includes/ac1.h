// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/ac1.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_AC1_H
#define MAME_INCLUDES_AC1_H

#pragma once

#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "emupal.h"

class ac1_state : public driver_device
{
public:
	ac1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cassette(*this, "cassette")
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_io_line(*this, "LINE.%u", 0)
	{ }

	void init_ac1();
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_ac1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ac1_32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(ac1_port_b_r);
	DECLARE_READ8_MEMBER(ac1_port_a_r);
	DECLARE_WRITE8_MEMBER(ac1_port_a_w);
	DECLARE_WRITE8_MEMBER(ac1_port_b_w);

	void ac1_32(machine_config &config);
	void ac1(machine_config &config);
	void ac1_32_mem(address_map &map);
	void ac1_io(address_map &map);
	void ac1_mem(address_map &map);
private:
	required_device<cassette_image_device> m_cassette;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport_array<7> m_io_line;
};

/*----------- defined in video/ac1.c -----------*/
extern const gfx_layout ac1_charlayout;

#endif // MAME_INCLUDES_AC1_H
