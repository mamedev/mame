// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Sal and John Bugliarisi,Paul Priest
#ifndef MAME_INCLUDES_NAUGHTYB_H
#define MAME_INCLUDES_NAUGHTYB_H

#pragma once

#include "audio/pleiads.h"
#include "emupal.h"
#include "screen.h"

class naughtyb_state : public driver_device
{
public:
	naughtyb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_naughtyb_custom(*this, "naughtyb_custom"),
		m_popflame_custom(*this, "popflame_custom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_scrollreg(*this, "scrollreg")
	{ }

	void naughtyb_base(machine_config &config);
	void popflame(machine_config &config);
	void naughtyb(machine_config &config);

	void init_trvmstr();
	void init_popflame();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<naughtyb_sound_device> m_naughtyb_custom;
	optional_device<popflame_sound_device> m_popflame_custom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_scrollreg;

	uint8_t m_popflame_prot_seed;
	int m_r_index;
	int m_prot_count;
	int m_question_offset;
	int m_cocktail;
	uint8_t m_palreg;
	int m_bankreg;
	bitmap_ind16 m_tmpbitmap;

	DECLARE_READ8_MEMBER(in0_port_r);
	DECLARE_READ8_MEMBER(dsw0_port_r);
	DECLARE_READ8_MEMBER(popflame_protection_r);
	DECLARE_WRITE8_MEMBER(popflame_protection_w);
	DECLARE_READ8_MEMBER(trvmstr_questions_r);
	DECLARE_WRITE8_MEMBER(trvmstr_questions_w);
	DECLARE_WRITE8_MEMBER(naughtyb_videoreg_w);
	DECLARE_WRITE8_MEMBER(popflame_videoreg_w);

	void naughtyb_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void naughtyb_map(address_map &map);
	void popflame_map(address_map &map);
};

#endif // MAME_INCLUDES_NAUGHTYB_H
