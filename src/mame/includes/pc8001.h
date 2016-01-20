// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PC8001__
#define __PC8001__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8257.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/upd1990a.h"
#include "sound/speaker.h"
#include "video/upd3301.h"

#define Z80_TAG         "z80"
#define I8251_TAG       "i8251"
#define I8255A_TAG      "i8255"
#define I8257_TAG       "i8257"
#define UPD1990A_TAG    "upd1990a"
#define UPD3301_TAG     "upd3301"
#define CENTRONICS_TAG  "centronics"
#define SCREEN_TAG      "screen"

class pc8001_state : public driver_device
{
public:
	pc8001_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_rtc(*this, UPD1990A_TAG),
			m_dma(*this, I8257_TAG),
			m_crtc(*this, UPD3301_TAG),
			m_cassette(*this, "cassette"),
			m_centronics(*this, CENTRONICS_TAG),
			m_cent_data_out(*this, "cent_data_out"),
			m_speaker(*this, "speaker"),
			m_ram(*this, RAM_TAG),
			m_rom(*this, Z80_TAG),
			m_char_rom(*this, UPD3301_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	required_device<i8257_device> m_dma;
	required_device<upd3301_device> m_crtc;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_char_rom;

	virtual void machine_start() override;

	DECLARE_WRITE8_MEMBER( port10_w );
	DECLARE_WRITE8_MEMBER( port30_w );
	DECLARE_READ8_MEMBER( port40_r );
	DECLARE_WRITE8_MEMBER( port40_w );
	DECLARE_WRITE_LINE_MEMBER( hrq_w );
	DECLARE_READ8_MEMBER( dma_mem_r );

	/* video state */
	int m_width80;
	int m_color;

	int m_centronics_busy;
	int m_centronics_ack;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
	UPD3301_DRAW_CHARACTER_MEMBER( pc8001_display_pixels );
};

class pc8001mk2_state : public pc8001_state
{
public:
	pc8001mk2_state(const machine_config &mconfig, device_type type, std::string tag)
		: pc8001_state(mconfig, type, tag),
			m_kanji_rom(*this, "kanji")
	{ }

	required_memory_region m_kanji_rom;

	DECLARE_WRITE8_MEMBER( port31_w );
};

#endif
