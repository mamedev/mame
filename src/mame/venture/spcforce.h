// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_SPCFORCE_H
#define MAME_INCLUDES_SPCFORCE_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/74259.h"
#include "sound/sn76496.h"
#include "emupal.h"

class spcforce_state : public driver_device
{
public:
	spcforce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sn(*this, "sn%u", 1U),
		m_scrollram(*this, "scrollram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void meteors(machine_config &config);
	void spcforce(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void sn76496_latch_w(uint8_t data);
	uint8_t sn76496_select_r();
	void sn76496_select_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(write_sn1_ready);
	DECLARE_WRITE_LINE_MEMBER(write_sn2_ready);
	DECLARE_WRITE_LINE_MEMBER(write_sn3_ready);
	DECLARE_READ_LINE_MEMBER(t0_r);
	void soundtrigger_w(uint8_t data);
	void misc_outputs_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE_LINE_MEMBER(unknown_w);

	void spcforce_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void spcforce_map(address_map &map);
	void spcforce_sound_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	required_device<i8035_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<sn76496_device, 3> m_sn;

	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	output_finder<2> m_lamps;

	int m_sn76496_latch = 0;
	int m_sn76496_select = 0;
	int m_sn1_ready = 0;
	int m_sn2_ready = 0;
	int m_sn3_ready = 0;
};

#endif // MAME_INCLUDES_SPCFORCE_H
