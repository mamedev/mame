// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
#ifndef MAME_IGS_PGM_H
#define MAME_IGS_PGM_H

#pragma once

#include "igs025.h"
#include "igs022.h"
#include "igs023_video.h"
#include "igs028.h"

#include "pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/v3021.h"
#include "sound/ics2115.h"
#include "emupal.h"
#include "tilemap.h"


class pgm_state : public driver_device
{
public:
	pgm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mainram(*this, "sram")
		, m_region(*this, "Region")
		, m_regionhack(*this, "RegionHack")
		, m_maincpu(*this, "maincpu")
		, m_z80_mainram(*this, "z80_mainram")
		, m_soundcpu(*this, "soundcpu")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch3(*this, "soundlatch3")
		, m_ics(*this, "ics")
		, m_video(*this, "igs023")
	{
		m_irq4_disabled = 0;
	}

	void init_pgm();

	void pgm_basic_init(bool set_bank = true);
	void pgm(machine_config &config);
	void pgmbase(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* memory pointers */
	required_shared_ptr<u16> m_mainram;

	optional_ioport m_region;
	optional_ioport m_regionhack;

	/* devices */
	required_device<cpu_device> m_maincpu;

	/* hack */
	int m_irq4_disabled = 0;

	void pgm_base_mem(address_map &map) ATTR_COLD;
	void pgm_mem(address_map &map) ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<u8>  m_z80_mainram;



	/* devices */
	required_device<cpu_device>             m_soundcpu;
	required_device<palette_device>         m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<ics2115_device>         m_ics;
	required_device<igs023_video_device>    m_video;

	/* used by rendering */
	void coin_counter_w(u16 data);
	u8 z80_ram_r(offs_t offset);
	void z80_ram_w(offs_t offset, u8 data);
	void z80_reset_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void z80_ctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void m68k_l1_w(u8 data);
	void z80_l3_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	u16 sprites_r(offs_t offset);

	void pgm_basic_mem(address_map &map) ATTR_COLD;
	void pgm_z80_io(address_map &map) ATTR_COLD;
	void pgm_z80_mem(address_map &map) ATTR_COLD;
};



/*----------- defined in drivers/pgm.cpp -----------*/

INPUT_PORTS_EXTERN(pgm);

extern gfx_decode_entry const gfx_pgm[];

#endif // MAME_IGS_PGM_H
