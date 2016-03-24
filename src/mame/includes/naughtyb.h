// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Sal and John Bugliarisi,Paul Priest
#include "audio/pleiads.h"

class naughtyb_state : public driver_device
{
public:
	naughtyb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_naughtyb_custom(*this, "naughtyb_custom"),
		m_popflame_custom(*this, "popflame_custom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_scrollreg(*this, "scrollreg") { }

	required_device<cpu_device> m_maincpu;
	optional_device<naughtyb_sound_device> m_naughtyb_custom;
	optional_device<popflame_sound_device> m_popflame_custom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_scrollreg;

	UINT8 m_popflame_prot_seed;
	int m_r_index;
	int m_prot_count;
	int m_question_offset;
	int m_cocktail;
	UINT8 m_palreg;
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

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	DECLARE_DRIVER_INIT(trvmstr);
	DECLARE_DRIVER_INIT(popflame);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(naughtyb);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
