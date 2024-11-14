// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Sal and John Bugliarisi,Paul Priest
#ifndef MAME_PHOENIX_NAUGHTYB_H
#define MAME_PHOENIX_NAUGHTYB_H

#pragma once

#include "pleiads.h"
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
	virtual void video_start() override ATTR_COLD;

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

	uint8_t m_popflame_prot_seed = 0;
	int m_r_index = 0;
	int m_prot_count = 0;
	int m_question_offset = 0;
	int m_cocktail = 0;
	uint8_t m_palreg = 0;
	int m_bankreg = 0;
	bitmap_ind16 m_tmpbitmap;

	uint8_t in0_port_r();
	uint8_t dsw0_port_r();
	uint8_t popflame_protection_r();
	void popflame_protection_w(uint8_t data);
	uint8_t trvmstr_questions_r();
	void trvmstr_questions_w(offs_t offset, uint8_t data);
	void naughtyb_videoreg_w(uint8_t data);
	void popflame_videoreg_w(uint8_t data);

	void naughtyb_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void naughtyb_map(address_map &map) ATTR_COLD;
	void popflame_map(address_map &map) ATTR_COLD;
};

#endif // MAME_PHOENIX_NAUGHTYB_H
