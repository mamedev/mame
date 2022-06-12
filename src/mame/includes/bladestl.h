// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Blades of Steel

*************************************************************************/
#ifndef MAME_INCLUDES_BLADESTL_H
#define MAME_INCLUDES_BLADESTL_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/flt_rc.h"
#include "sound/upd7759.h"
#include "video/k007342.h"
#include "video/k007420.h"
#include "video/k051733.h"
#include "emupal.h"

class bladestl_state : public driver_device
{
public:
	bladestl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_upd7759(*this, "upd"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_trackball(*this, "TRACKBALL.%u", 0),
		m_rombank(*this, "rombank"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	/* devices */
	uint8_t trackball_r(offs_t offset);
	void bladestl_bankswitch_w(uint8_t data);
	void bladestl_port_B_w(uint8_t data);
	uint8_t bladestl_speech_busy_r();
	void bladestl_speech_ctrl_w(uint8_t data);
	void bladestl_palette(palette_device &palette) const;
	uint32_t screen_update_bladestl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bladestl_scanline);
	K007342_CALLBACK_MEMBER(bladestl_tile_callback);
	K007420_CALLBACK_MEMBER(bladestl_sprite_callback);
	void bladestl(machine_config &config);
	void main_map(address_map &map);
	void sound_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	required_device<upd7759_device> m_upd7759;
	required_device<filter_rc_device> m_filter1;
	required_device<filter_rc_device> m_filter2;
	required_device<filter_rc_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;
	required_ioport_array<4> m_trackball;

	/* memory pointers */
	required_memory_bank m_rombank;

	/* video-related */
	int        m_spritebank = 0;

	/* misc */
	int        m_last_track[4]{};

	output_finder<2> m_lamps;
};

#endif // MAME_INCLUDES_BLADESTL_H
