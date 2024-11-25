// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/
#ifndef MAME_ZACCARIA_LASERBAT_H
#define MAME_ZACCARIA_LASERBAT_H

#pragma once

#include "zaccaria_a.h"

#include "cpu/s2650/s2650.h"

#include "machine/6821pia.h"
#include "machine/pla.h"
#include "machine/s2636.h"

#include "sound/ay8910.h"
#include "sound/sn76477.h"
#include "sound/tms3615.h"

#include "emupal.h"
#include "screen.h"


class laserbat_state_base : public driver_device
{
public:
	void laserbat_base(machine_config &config);
	void laserbat_io_map(address_map &map) ATTR_COLD;
	void laserbat_map(address_map &map) ATTR_COLD;

protected:
	laserbat_state_base(const machine_config &mconfig, device_type type, const char *tag, uint8_t eff2_mask)
		: driver_device(mconfig, type, tag)
		, m_mux_ports(*this, {"ROW0", "ROW1", "SW1", "SW2"})
		, m_row1(*this, "ROW1")
		, m_row2(*this, "ROW2")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_gfxmix(*this, "gfxmix")
		, m_pvi(*this, "pvi%u", 1U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_gfx1(*this, "gfx1")
		, m_gfx2(*this, "gfx2")
		, m_eff2_mask(eff2_mask)
	{
	}

	// control ports
	void ct_io_w(uint8_t data);
	uint8_t rrowx_r();

	INTERRUPT_GEN_MEMBER(laserbat_interrupt);

	// video memory and control ports
	void videoram_w(offs_t offset, uint8_t data);
	void wcoh_w(uint8_t data);
	void wcov_w(uint8_t data);
	void cnt_eff_w(uint8_t data);
	void cnt_nav_w(uint8_t data);

	// sound control ports
	virtual uint8_t rhsc_r();
	virtual void whsc_w(uint8_t data);
	virtual void csound1_w(uint8_t data);
	virtual void csound2_w(uint8_t data);

	// running the video
	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update_laserbat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// video functions
	TIMER_CALLBACK_MEMBER(video_line);

	// input lines
	required_ioport_array<4>                m_mux_ports;
	required_ioport                         m_row1;
	required_ioport                         m_row2;

	// main CPU device
	required_device<s2650_device>           m_maincpu;

	// video devices
	required_device<screen_device>          m_screen;
	required_device<palette_device>         m_palette;
	required_device<pla_device>             m_gfxmix;
	required_device_array<s2636_device, 3>  m_pvi;
	required_device<gfxdecode_device>       m_gfxdecode;

	// stuff for rendering video
	required_region_ptr<uint8_t>            m_gfx1;
	required_region_ptr<uint8_t>            m_gfx2;
	emu_timer                               *m_scanline_timer = nullptr;
	bitmap_ind16                            m_bitmap;
	uint16_t                                m_gfx2_base = 0;
	uint8_t const                           m_eff2_mask;

	// control lines
	uint8_t         m_input_mux = 0;
	bool            m_mpx_p_1_2 = 0;

	// RAM used by TTL video hardware, writable by CPU
	uint8_t         m_bg_ram[0x400]{};        // background tilemap
	uint8_t         m_eff_ram[0x400]{};       // per-scanline effects (A8 not wired meaning only half is usable)
	bool            m_mpx_bkeff = false;    // select between writing background and effects memory

	// signals affecting the TTL-generated 32x32 sprite
	bool            m_nave = false;         // 1-bit enable
	uint8_t         m_clr_lum = 0;          // 3-bit colour/luminance
	uint8_t         m_shp = 0;              // 3-bit shape
	uint8_t         m_wcoh = 0;             // 8-bit offset horizontal
	uint8_t         m_wcov = 0;             // 8-bit offset vertical

	// video effects signals
	bool            m_abeff1 = false;       // 1-bit effect enable
	bool            m_abeff2 = false;       // 1-bit effect enable
	bool            m_mpx_eff2_sh = false;  // 1-bit effect selection
	uint8_t         m_coleff = 0;           // 2-bit colour effect
	bool            m_neg1 = false;         // 1-bit area selection
	bool            m_neg2 = false;         // 1-bit area selection

	// sound board I/O signals
	uint8_t         m_rhsc = 0;             // 8-bit input from J7
	uint8_t         m_whsc = 0;             // 8-bit output to J7
	uint8_t         m_csound1 = 0;          // bits 1-8 on J3
	uint8_t         m_csound2 = 0;          // bits 9-16 on J3
};


class laserbat_state : public laserbat_state_base
{
public:
	laserbat_state(const machine_config &mconfig, device_type type, const char *tag)
		: laserbat_state_base(mconfig, type, tag, 0x00)
		, m_csg(*this, "csg")
		, m_synth_low(*this, "synth_low")
		, m_synth_high(*this, "synth_high")
	{
	}

	void laserbat(machine_config &config);

protected:
	// initialisation/startup
	virtual void machine_start() override ATTR_COLD;

	// video initialisation
	void laserbat_palette(palette_device &palette) const;

	// sound control ports
	virtual void csound2_w(uint8_t data) override;

	// sound board devices
	required_device<sn76477_device> m_csg;
	required_device<tms3615_device> m_synth_low;
	required_device<tms3615_device> m_synth_high;

	// register state
	uint32_t    m_keys = 0; // low octave keys 1-13 and high octave keys 2-12 (24 bits)
};


class catnmous_state : public laserbat_state_base
{
public:
	catnmous_state(const machine_config &mconfig, device_type type, const char *tag)
		: laserbat_state_base(mconfig, type, tag, 0x03)
		, m_audiopcb(*this, "audiopcb")
	{
	}

	void catnmous(machine_config &config);

protected:

	// video initialisation
	void catnmous_palette(palette_device &palette) const;

	// sound control ports
	virtual void csound1_w(uint8_t data) override;
	virtual void csound2_w(uint8_t data) override;

	required_device<zac1b11107_audio_device>    m_audiopcb;
};

#endif // MAME_ZACCARIA_LASERBAT_H
