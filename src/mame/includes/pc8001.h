// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese

#pragma once

#ifndef MAME_INCLUDES_PC8001_H
#define MAME_INCLUDES_PC8001_H

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8257.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/upd1990a.h"
#include "machine/pc80s31k.h"
#include "sound/beep.h"
#include "video/upd3301.h"
#include "emupal.h"
#include "screen.h"

#define Z80_TAG         "z80"
#define N80SR_ROM_TAG   "n80sr_rom"
#define I8251_TAG       "i8251"
#define I8257_TAG       "i8257"
#define UPD1990A_TAG    "upd1990a"
#define UPD3301_TAG     "upd3301"
#define CGROM_TAG       "cgrom"
#define CENTRONICS_TAG  "centronics"

class pc8001_base_state : public driver_device
{
public:
	pc8001_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_crtc(*this, UPD3301_TAG)
		, m_crtc_palette(*this, "crtc_palette")
		, m_dma(*this, I8257_TAG)
		, m_cassette(*this, "cassette")
		, m_cgrom(*this, CGROM_TAG)
	{}

protected:
	required_device<cpu_device> m_maincpu;
	required_device<upd3301_device> m_crtc;
	required_device<palette_device> m_crtc_palette;
	required_device<i8257_device> m_dma;
	required_device<cassette_image_device> m_cassette;
	required_memory_region m_cgrom;

	void port30_w(u8 data);
	virtual void machine_start() override;
	void set_screen_frequency(bool is_24KHz) { m_screen_is_24KHz = is_24KHz; }

	DECLARE_WRITE_LINE_MEMBER( crtc_reverse_w );
	UPD3301_DRAW_CHARACTER_MEMBER( draw_text );
	UPD3301_FETCH_ATTRIBUTE( attr_fetch );
	DECLARE_WRITE_LINE_MEMBER( hrq_w );
	uint8_t dma_mem_r(offs_t offset);

private:
	bool m_screen_reverse;
	bool m_screen_is_24KHz;

	/* video state */
	int m_width80;
	int m_color;
};

class pc8001_state : public pc8001_base_state
{
public:
	pc8001_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001_base_state(mconfig, type, tag)
		, m_pc80s31(*this, "pc80s31")
		, m_rtc(*this, UPD1990A_TAG)
		, m_screen(*this, "screen")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_cent_data_out(*this, "cent_data_out")
		, m_beep(*this, "beeper")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, Z80_TAG)
	{ }

	void pc8001(machine_config &config);

protected:
	void pc8001_io(address_map &map);
	void pc8001_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<pc80s31_device> m_pc80s31;
	required_device<upd1990a_device> m_rtc;
	required_device<screen_device> m_screen;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<beep_device> m_beep;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

private:
	void port10_w(uint8_t data);
	uint8_t port40_r();
	void port40_w(uint8_t data);

	int m_centronics_busy;
	int m_centronics_ack;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
};

class pc8001mk2_state : public pc8001_state
{
public:
	pc8001mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001_state(mconfig, type, tag)
		, m_kanji_rom(*this, "kanji")
		, m_dsw(*this, "DSW%d", 1U)
	{ }

	void pc8001mk2(machine_config &config);

protected:
	void pc8001mk2_io(address_map &map);
	void pc8001mk2_map(address_map &map);

	required_memory_region m_kanji_rom;
	required_ioport_array<2> m_dsw;
private:
	void port31_w(uint8_t data);
};

class pc8001mk2sr_state : public pc8001mk2_state
{
public:
	pc8001mk2sr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001mk2_state(mconfig, type, tag)
		, m_n80sr_rom(*this, N80SR_ROM_TAG)
	{ }

	void pc8001mk2sr(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void pc8001mk2sr_io(address_map &map);

	required_memory_region m_n80sr_rom;

	void port33_w(u8 data);
	u8 port71_r();
	void port71_w(u8 data);

	u8 m_n80sr_bank;
};

#endif
