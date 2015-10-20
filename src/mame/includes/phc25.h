// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
#pragma once

#ifndef __PHC25__
#define __PHC25__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "machine/ram.h"
#include "bus/centronics/ctronics.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"
#include "formats/phc25_cas.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define AY8910_TAG      "ay8910"
#define MC6847_TAG      "mc6847"
#define CENTRONICS_TAG  "centronics"

#define PHC25_VIDEORAM_SIZE     0x1800

class phc25_state : public driver_device
{
public:
	phc25_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video_ram(*this, "video_ram")
		, m_maincpu(*this, Z80_TAG)
		, m_vdg(*this, MC6847_TAG)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_cassette(*this, "cassette")
	{ }

	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_READ8_MEMBER( port40_r );
	DECLARE_WRITE8_MEMBER( port40_w );
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_READ8_MEMBER( video_ram_r );
	MC6847_GET_CHARROM_MEMBER(ntsc_char_rom_r);
	MC6847_GET_CHARROM_MEMBER(pal_char_rom_r);

private:
	virtual void video_start();
	UINT8 *m_char_rom;
	UINT8 m_port40;
	required_shared_ptr<UINT8> m_video_ram;
	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
	int m_centronics_busy;
};

#endif
