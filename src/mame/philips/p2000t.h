// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/*****************************************************************************
 *
 * includes/p2000t.h
 *
 ****************************************************************************/

#ifndef MAME_PHILIPS_P2000T_H
#define MAME_PHILIPS_P2000T_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "video/saa5050.h"
#include "machine/mdcr.h"
#include "machine/ram.h"
#include "emupal.h"


class p2000t_state : public driver_device
{
public:
	p2000t_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_mdcr(*this, "mdcr")
		, m_ram(*this, RAM_TAG)
		, m_bank(*this, "bank")
		, m_keyboard(*this, "KEY.%u", 0)
	{
	}

	void p2000t(machine_config &config);

protected:
	uint8_t p2000t_port_000f_r(offs_t offset);
	uint8_t p2000t_port_202f_r();
	void p2000t_port_101f_w(uint8_t data);
	void p2000t_port_303f_w(uint8_t data);
	void p2000t_port_505f_w(uint8_t data);
	void p2000t_port_707f_w(uint8_t data);
	void p2000t_port_888b_w(uint8_t data);
	void p2000t_port_8c90_w(uint8_t data);
	void p2000t_port_9494_w(uint8_t data);
	uint8_t videoram_r(offs_t offset);
	virtual void machine_start() override ATTR_COLD;

	INTERRUPT_GEN_MEMBER(p2000_interrupt);

	void p2000t_mem(address_map &map) ATTR_COLD;
	void p2000t_io(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<mdcr_device> m_mdcr;
	required_device<ram_device> m_ram;
	required_memory_bank m_bank;

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
	virtual void video_start() override ATTR_COLD;
	void p2000m_palette(palette_device &palette) const;
	uint32_t screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void p2000m_mem(address_map &map) ATTR_COLD;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int8_t m_frame_count = 0;
};

#endif // MAME_PHILIPS_P2000T_H
