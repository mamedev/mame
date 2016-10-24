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

	uint8_t in0_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw0_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t popflame_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void popflame_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t trvmstr_questions_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void trvmstr_questions_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void naughtyb_videoreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void popflame_videoreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void init_trvmstr();
	void init_popflame();
	virtual void video_start() override;
	void palette_init_naughtyb(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
