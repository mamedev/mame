// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/*****************************************************************************
 *
 * includes/p2000t.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_P2000T_H
#define MAME_INCLUDES_P2000T_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "video/saa5050.h"
#include "emupal.h"


class p2000t_state : public driver_device
{
public:
	p2000t_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_keyboard(*this, "KEY.%u", 0)
	{
	}

	void p2000t(machine_config &config);

protected:
	DECLARE_READ8_MEMBER(p2000t_port_000f_r);
	DECLARE_READ8_MEMBER(p2000t_port_202f_r);
	DECLARE_WRITE8_MEMBER(p2000t_port_101f_w);
	DECLARE_WRITE8_MEMBER(p2000t_port_303f_w);
	DECLARE_WRITE8_MEMBER(p2000t_port_505f_w);
	DECLARE_WRITE8_MEMBER(p2000t_port_707f_w);
	DECLARE_WRITE8_MEMBER(p2000t_port_888b_w);
	DECLARE_WRITE8_MEMBER(p2000t_port_8c90_w);
	DECLARE_WRITE8_MEMBER(p2000t_port_9494_w);
	DECLARE_READ8_MEMBER(videoram_r);

	INTERRUPT_GEN_MEMBER(p2000_interrupt);

	void p2000t_mem(address_map &map);
	void p2000t_io(address_map &map);

	required_shared_ptr<uint8_t> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

private:
	required_ioport_array<10> m_keyboard;
	uint8_t m_port_101f;
	uint8_t m_port_202f;
	uint8_t m_port_303f;
	uint8_t m_port_707f;
};

class p2000m_state : public p2000t_state
{
public:
	p2000m_state(const machine_config &mconfig, device_type type, const char *tag)
		: p2000t_state(mconfig, type, tag)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{
	}

	void p2000m(machine_config &config);

protected:
	virtual void video_start() override;
	void p2000m_palette(palette_device &palette) const;
	uint32_t screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void p2000m_mem(address_map &map);

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int8_t m_frame_count;
};

#endif // MAME_INCLUDES_P2000T_H
